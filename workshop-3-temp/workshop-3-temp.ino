/**
 * Workshop example for periodically sending temperature data.
 * 
 * Visit http://www.getstructure.io/kit for full instructions.
 *
 * Copyright (c) 2016 Structure. All rights reserved.
 * http://www.getstructure.io
 */

#include <ESP8266WiFi.h>
#include <Structure.h>

// WiFi credentials.
const char* WIFI_SSID = "my-wifi-ssid";
const char* WIFI_PASS = "my-wifi-pass";

// Structure credentials.
const char* DEVICE_ID = "my-device-id";
const char* ACCESS_KEY = "my-access-key";
const char* ACCESS_SECRET = "my-access-secret";

const int BUTTON_PIN = 14;
const int LED_PIN = 12;

bool ledState = false;

WiFiClientSecure wifiClient;

StructureDevice device(DEVICE_ID);

void toggle() {
  Serial.println("Toggling LED.");
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void handleCommand(StructureCommand *command) {
  Serial.print("Command received: ");
  Serial.println(command->name);
  
  if(strcmp(command->name, "toggle") == 0) {
    toggle(); 
  }
}

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.print("Connecting to Structure...");

  device.connectSecure(wifiClient, ACCESS_KEY, ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected!");
}
 
void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  device.onCommand(&handleCommand);
  connect();
}

void buttonPressed() {
  Serial.println("Button Pressed!");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["button"] = true;
  device.sendState(root);
}

void reportTemp(double degreesC, double degreesF) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["tempC"] = degreesC;
  root["tempF"] = degreesF;
  device.sendState(root);
}

int buttonState = 0;

int timeSinceLastRead = 0;
int tempSum = 0;
int tempCount = 0;
 
void loop() {

  bool toReconnect = false;

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if(!device.connected()) {
    Serial.println("Disconnected from MQTT");
    Serial.println(device.mqttClient.state());
    toReconnect = true;
  }
  
  if(toReconnect) {
    connect();
  }

  device.loop();

  int currentRead = digitalRead(BUTTON_PIN);

  if(currentRead != buttonState) {
    buttonState = currentRead;
    if(buttonState) {
      buttonPressed();
    }
  }

  tempSum += analogRead(A0);
  tempCount++; 

  // Report every 15 seconds.
  if(timeSinceLastRead > 15000) {
    // Take the average reading over the last 15 seconds.
    double raw = (double)tempSum / (double)tempCount;
    double degreesC = (((raw / 1024.0) * 2.0) - 0.5) * 100.0;
    double degreesF = degreesC * 1.8 + 32;

    Serial.println();
    Serial.print("Temperature C: ");
    Serial.println(degreesC);
    Serial.print("Temperature F: ");
    Serial.println(degreesF);
    Serial.println();

    reportTemp(degreesC, degreesF);
    
    timeSinceLastRead = 0;
    tempSum = 0;
    tempCount = 0;
  }

  delay(100);
  timeSinceLastRead += 100;
}


