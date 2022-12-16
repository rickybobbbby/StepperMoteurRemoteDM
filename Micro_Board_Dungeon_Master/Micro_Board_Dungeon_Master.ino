/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/stepper-motor-esp32-websocket/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Stepper.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 15
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

String message = "";

// Replace with your network credentials
const char* ssid = "Yannick96";
const char* password = "1234yann";
//const char* ssid = "reseau2";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

//Variables to save values from HTML form
String direction ="Start";
String steps;

String actionType ="None";
int r;
String DiceSteps;
int DiceSection;

bool newRequest = false;

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else{
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notifyClients(String state) {
  ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    steps = message.substring(message.indexOf("Steps")+5, message.indexOf("ActionType"));
    actionType = message.substring(message.indexOf("ActionType")+10, message.indexOf("Direction"));
    direction = message.substring(message.indexOf("Direction")+9, message.length());
    
    Serial.print("steps");
    Serial.println(steps);
    
    Serial.print("Action Type");
    Serial.println(actionType);
    
    Serial.print("direction");
    Serial.println(direction);
    notifyClients(direction);
    newRequest = true;
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      //Notify client of motor current state when it first connects
      notifyClients(direction);
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

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  // Serial port for debugging purposes

  Serial.begin(115200);
  initWiFi();
  initWebSocket();
  initSPIFFS();
  myStepper.setSpeed(12);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");

  server.begin();
}


void loop() {
  if (newRequest){
    if (actionType == "1d4"){
      DiceSection = 2048 / 4;
      Serial.print((int)DiceSection);
      r = rand() % 4;
      Serial.print((int)r);
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 4");
      notifyClients(notifyString);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    if (actionType == "1d6"){
      DiceSection = 2048 / 6;
      Serial.print((int)DiceSection);
      r = rand() % 6;
      Serial.print((int)r);
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 6");
      notifyClients(notifyString);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    if (actionType == "1d8"){
      DiceSection = 2048 / 8;
      Serial.print((int)DiceSection);
      r = rand() % 8;
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 8");
      notifyClients(notifyString);
      Serial.print((int)r);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    if (actionType == "1d20"){
      DiceSection = 2048 / 20;
      Serial.print((int)DiceSection);
      r = rand() % 20;
       String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 20");
      notifyClients(notifyString);
      Serial.print((int)r);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    
    if (direction == "CW"){
      myStepper.step(steps.toInt());
      delay(1000);
      myStepper.step(-steps.toInt());
      Serial.print("CW");
    }
    if (direction == "CCW"){
      myStepper.step(-steps.toInt());
      delay(1000);
      myStepper.step(steps.toInt());
      Serial.print("CCW");
    }
    delay(1000);
    newRequest = false;
    notifyClients("stop");
  }
  ws.cleanupClients();
}
