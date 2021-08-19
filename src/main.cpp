/*
  Rui Santos
  Complete project details
   - Arduino IDE: https://RandomNerdTutorials.com/esp32-ota-over-the-air-arduino/
   - VS Code: https://RandomNerdTutorials.com/esp32-ota-over-the-air-vs-code/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
Adafruit_ADS1115 ads;


// Set your Static IP address
// IPAddress Ip(172, 20, 10, 7);
// IPAddress gateway(172, 20, 10, 1);
// IPAddress subnet(255, 255, 255, 240);
// IPAddress dns(172, 20, 10, 1);

// Replace with your network credentials
const char *ssid = "Meriem's iphone";
const char *password = "meriemsg98";

const char *host = "emoncms.org";
String sensorName = "essai";
String apikey = "c61d0e426d578d68fc7908693fd13077";

unsigned long tim1, tim2, now;
unsigned long lasttim, elapsed;
unsigned long lastsec = 0;
int counter = 0;
double totalPower = 0;
double Asum = 0;
double Vsum = 0;
double WuSsum = 0;
double totalSecs = 0;
double sensitivity = 0.066;
int numzeros = 0;
double V,A, W, WuS, adcVoltage;
double avgV = Vsum / counter;
double avgA = Asum / counter;


WiFiClient client;
const int httpPort = 80;

const long msgTimeoutLimit = 10000; // 5 seconds
const long connectionTimeoutLimit = 30000; // 30 seconds
unsigned long msgTimeout = millis();

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Set number of outputs
#define NUM_OUTPUTS 4

// Assign each GPIO to an output:
int outputGPIOs[NUM_OUTPUTS] = {2, 4, 12, 14};

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
  WiFi.softAP("OOSP", "justepourtester");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  unsigned long connectionTimeout = millis(); 
  while (WiFi.status() != WL_CONNECTED && millis() - connectionTimeout < connectionTimeoutLimit )
  {
    Serial.print('.');
    delay(1000);
  }
  if(millis() - connectionTimeout > connectionTimeoutLimit){
    Serial.println("Connection failed");
  }else{
    Serial.println(WiFi.localIP());
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

  server.serveStatic("/", SPIFFS, "/");

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);

  // Start server
  server.begin();
}

void loop()
{
  AsyncElegantOTA.loop();
  ws.cleanupClients();

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
    tim2 = micros();
    int16_t adc1;
    int16_t adc0;  
    adc1 = ads.readADC_SingleEnded(1);
    adc0 = ads.readADC_SingleEnded(0);

    V = ads.computeVolts(adc1);
    Vsum += V;

    adcVoltage = ads.computeVolts(adc0);
    A =adcVoltage / sensitivity ;
    Asum += A;

    W = A * V;
    WuS = W * elapsed;
    WuSsum += WuS;


    if (V <= 0) numzeros++; else numzeros=0;
    if (numzeros > 2) 
    {
      Vsum = 0;
      counter = 0;
      Asum = 0;
      WuSsum = 0;
      totalPower = 0;
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
      double WH = WuSsum / 3600000000; // divide by uS / H
      totalPower += WH;

      totalSecs += lastsec;

      Serial.print(counter);
      Serial.print(" V: ");
      Serial.print(avgV, 7);
      Serial.print(" A: ");
      Serial.print(avgA, 7);
      Serial.print(" W: ");
      Serial.print(avgV * avgA, 7);
      Serial.print(" P: ");
      Serial.print(totalPower,7);
      Serial.print(" S: ");
      Serial.print(totalSecs / 1000000);
      Serial.println();
     }
    } 
    
      String url = "https://emoncms.org";
      url += "/input/post?node=";
      url += sensorName;
      url += "&json={'V':";
      url += avgV;
      url += ",'A':";
      url += avgA;
      url += ",'W':";
      url += avgV * avgA;
      url += ",'P':";
      url += totalPower;
      url += ",'S':";
      url += totalSecs/1000000;
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
      msgTimeout = millis();
      
      Vsum = 0;
      counter = 0;
      Asum = 0;
      WuSsum = 0;
      lastsec = 0;
     
  }
}
