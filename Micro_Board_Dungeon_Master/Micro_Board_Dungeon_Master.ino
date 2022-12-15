/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/stepper-motor-esp32-web-server/
  
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


// Stepper Motor Settings
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 15
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// Replace with your network credentials
const char* ssid = "Yannick96";
const char* password = "1234yann";

//const char* ssid = "reseau2";



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "direction";
const char* PARAM_INPUT_2 = "steps";

const char* PARAM_INPUT_3 = "Dice";

//Variables to save values from HTML form
String direction;
String Dice;
String steps;
int r;
String DiceSteps;
int DiceSection;

bool newRequest = false;

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else {
  Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
//WiFi.begin(ssid, password);
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

void setup() {
  // Serial port for debugging purposes

  Serial.begin(115200);
  initWiFi();
  initSPIFFS();
  myStepper.setSpeed(5);


  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST input1 value
        if (p->name() == PARAM_INPUT_1) {
          direction = p->value().c_str();
          Serial.print("Direction set to: ");
          Serial.println(direction);
        }
        // HTTP POST input1 value
        if (p->name() == PARAM_INPUT_3) {
          Dice = p->value().c_str();
          Serial.print("Le type de dés rouler est: ");
          Serial.println(Dice);
        }
        // HTTP POST input2 value
        if (p->name() == PARAM_INPUT_2) {
          steps = p->value().c_str();
          Serial.print("Number of steps set to: ");
          Serial.println(steps);
          // Write file to save value
        }
        newRequest = true;
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.begin();
}

void loop() {
  if (newRequest){
    if (Dice == "1d4"){
      DiceSection = 2048 / 4;
      Serial.print((int)DiceSection);
      r = rand() % 5;
      Serial.print((int)r);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    if (Dice == "1d6"){
      DiceSection = 2048 / 6;
      Serial.print((int)DiceSection);
      r = rand() % 7;
      Serial.print((int)r);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    if (Dice == "1d8"){
      DiceSection = 2048 / 8;
      Serial.print((int)DiceSection);
      r = rand() % 9;
      Serial.print((int)r);
      DiceSteps = ((int)DiceSection) * ((int)r);
      myStepper.step(DiceSteps.toInt());
      Serial.print((String)DiceSteps);
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    if (Dice == "1d20"){
      DiceSection = 2048 / 20;
      Serial.print((int)DiceSection);
      r = rand() % 21;
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
  }
}
