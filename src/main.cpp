// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
#include "EmonLib.h"

#include "SPIFFS.h"
#include "config.h"

// Set number of outputs
#define NUM_OUTPUTS 1

// Set your Static IP address
// IPAddress Ip(172, 20, 10, 7);
// IPAddress gateway(172, 20, 10, 1);
// IPAddress subnet(255, 255, 255, 240);
// IPAddress dns(172, 20, 10, 1);

// Replace with your network credentials
const char *ssid = "";
const char *password = "";

const char *host = "emoncms.org";
String id = String((uint32_t)ESP.getEfuseMac(), HEX);
String sensorName = "OSSP " + id;
String apikey = "c61d0e426d578d68fc7908693fd13077";

unsigned long tim1, tim2, now;
unsigned long lasttim, elapsed;
unsigned long lastsec = 0;
int counter = 0;

double Vsum = 0;
double totalSecs = 0;
int numzeros = 0;
double V;
// double avgV = Vsum / counter;

// Assign each GPIO to an output:
int outputGPIOs[NUM_OUTPUTS] = {2};

const int httpPort = 80;

const long msgTimeoutLimit = 10000;        // 5 seconds
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

  if (ssid != "")
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
  myArray["power"] = 10;
  myArray["current"] = 8;
  myArray["voltage"] = 8;
  myArray["powerfactor"] = 20;
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

  // sdefine parameters for energy monitoring lib
  emon1.current(1, 111.1);

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
}

void loop()
{
  AsyncElegantOTA.loop();
  ws.cleanupClients();

  // Measure the current
  double Irms = emon1.calcIrms(1480);

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

    tim1 = micros();
    if (!ads.begin())
    {
      int16_t adc0;
      float volts0;
      adc0 = ads.readADC_SingleEnded(0);
      volts0 = ads.computeVolts(adc0);
    }

    /*Vsum += V;*/
    if (V <= 0)
      numzeros++;
    else
      numzeros = 0;
    if (numzeros > 2)
    {
      Vsum = 0;
      counter = 0;
      totalSecs = 0;
      lastsec = 0;
      numzeros = 0;
    }
    else
    {
      counter++;
      now = (tim1 + tim2) / 2;
      elapsed = now - lasttim;
      lastsec += elapsed;
      lasttim = now;

      if (lastsec > 1000000)
      {
        // end of our second
        totalSecs += lastsec;

        Serial.print(counter);
        Serial.print(" V: ");
        Serial.print(0);
        // Serial.print(avgV, 7);
        Serial.print(" S: ");
        Serial.print(totalSecs / 1000000);
        Serial.println();
      }
    }

    // Send the message

    String url = "https://";
    url += host;
    url += "/input/post?node=";
    url += sensorName;
    url += "&json={'V':";
    url += 0;
    //url += avgV;
    url += ",'S':";
    url += totalSecs / 1000000;
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
    lastsec = 0;
  }
}
