// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
//#include "EmonLib.h"
#include "CustomEmonLib.h"

#include "SPIFFS.h"
#include "config.h"

// Set number of outputs
#define NUM_OUTPUTS 1

// 16 bits --> resolution 65536 --> 3300mV*65536

#define READVCC_CALIBRATION_CONST 216268800L

// Set your Static IP address
// IPAddress Ip(172, 20, 10, 7);
// IPAddress gateway(172, 20, 10, 1);
// IPAddress subnet(255, 255, 255, 240);
// IPAddress dns(172, 20, 10, 1);

// Replace with your network credentials
/* If using env file with credentials
const char *ssid = AP_SSID;
const char *password = AP_PASS;*/
const char *ssid = "";
const char *password = "";

// Open Energy Monitoring (EmonCMS) credentials
const char *host = "oem.lf2l.fr";
String apikey = "c2b8c4a5fc94e757e0d6853841d553f1";

// Default sensor name based on MAC adress
String id = String((uint32_t)ESP.getEfuseMac(), HEX);
String sensorName = "OSSP-" + id;

unsigned long tim1, tim2, now;
unsigned long lasttim, elapsed, lastScreenRefresh;
unsigned long lastsec = 0;
int counter = 0;
double totalPower = 0;
double Asum = 0;
double Vsum = 0;
double WuSsum = 0;
double totalSecs = 0;
double sensitivity = 0.066;
int numzeros = 0;
double V, A, W, WuS, adcVoltage; // calculation variable
double voltage, current, power, powerFactor;  // information displayed on the sensor dashboard

bool relayState = HIGH;
bool APconnected = LOW;

// Assign each GPIO to an output:
int outputGPIOs[NUM_OUTPUTS] = {2}; // LED on the ESP32 burner board
//int outputGPIOs[NUM_OUTPUTS] = {2}; // Relay on the board

const int httpPort = 80;

const long msgTimeoutLimit = 10000;        // send a message every 10 seconds
const long screenRefreshTimeout = 2000;     // refresh the client screen through the websocket every 2 seconds
const long connectionTimeoutLimit = 30000; // 30 seconds
unsigned long msgTimeout = millis();

//*** Init object ***//

EnergyMonitor emon1;

WiFiClient client;

Adafruit_ADS1115 ads;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(sensorName.c_str(), "justepourtester");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  if (sizeof(ssid) > 1)
  {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    unsigned long connectionTimeout = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectionTimeout < connectionTimeoutLimit)
    {
      Serial.print('.');
      delay(1000);
    }

    if (millis() - connectionTimeout > connectionTimeoutLimit)
    {
      Serial.println("Connection failed");
    }
    else
    {
      Serial.println(WiFi.localIP());
      APconnected = HIGH;
    }
  }
  else
  {
    Serial.println("No SSID provided by default.");
  }
}

String getOutputStates()
{
  JSONVar myArray;
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  myArray["APconnected"] = APconnected;
  myArray["ID"] = sensorName;
  myArray["power"] = power;
  myArray["current"] = current;
  myArray["voltage"] = voltage;
  myArray["powerfactor"] = powerFactor;
  String jsonString = JSON.stringify(myArray);
  return jsonString;
}

void notifyClients(String state)
{
  ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "states") == 0)
    {
      notifyClients(getOutputStates());
    }
    else
    {
      int gpio = atoi((char *)data);
      digitalWrite(gpio, !digitalRead(gpio));
      notifyClients(getOutputStates());
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  // define parameters for energy monitoring lib
  emon1.voltage(0, 234.26, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(1, 111.1);       // Current: input pin, calibration.

  // Set GPIOs as outputs
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    pinMode(outputGPIOs[i], OUTPUT);
  }

  initSPIFFS();
  initWiFi();
  initWebSocket();

  // Configures static IP address
  // if (!WiFi.config(Ip, gateway, subnet, dns))
  // {
  //   Serial.println("STA Failed to configure");
  // }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html", false); });
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/settings.html", "text/html", false); });

  server.serveStatic("/", SPIFFS, "/");

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);

  // Start server
  server.begin();

  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
  }

  lastScreenRefresh = millis();
}

void loop()
{
  ws.cleanupClients();

  // Measure the current
  emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out

  current = emon1.Irms; // average current
  voltage = emon1.Vrms; // average voltage
  power = emon1.realPower; // real power 
  powerFactor = emon1.powerFactor; 

  if (millis() - lastScreenRefresh > screenRefreshTimeout ){
    if(WiFi.status() == WL_CONNECTED){
      APconnected = HIGH;
    }else{
      APconnected = LOW;
    }
    
    notifyClients(getOutputStates());
    lastScreenRefresh = millis();
  }

    // if the device is connected to a network and reach the delay, send data to the server
  if (millis() - msgTimeout > msgTimeoutLimit && WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Testing host");
    if (!client.connect(host, httpPort))
    {
      Serial.println("Connection failed");
      return;
    }
    else
    {
      Serial.println("Connection OK");
    }

    // Send the message
    String url = "https://";
    url += host;
    url += "/input/post?node=";
    url += sensorName;
    url += "&json={'V':";
    url += emon1.Vrms;
    url += ",'A':";
    url += emon1.Irms;
    url += ",'W':";
    url += emon1.realPower;
/*     url += ",'P':";
    url += totalPower;
    url += ",'S':";
    url += totalSecs / 1000000; */
    url += "}&apikey=";
    url += apikey;

    Serial.print("Requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0)
    {
      if (millis() - timeout > 5000)
      {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Read all the lines of the reply from server and print them to Serial
    while (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");

    // reinitialise variables for the next loop
    msgTimeout = millis();

    Vsum = 0;
    counter = 0;
    Asum = 0;
    WuSsum = 0;
    lastsec = 0;
  }
}
