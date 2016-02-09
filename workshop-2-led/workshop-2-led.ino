/**
 * Workshop example for a remote control LED.
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

// For an unsecure connection to Structure.
WiFiClientSecure wifiClient;

StructureDevice device(DEVICE_ID);

void toggle() {
  Serial.println("Toggling LED.");
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

// Called whenever the device receives a command from the Structure platform.
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

  // Connect to Structure.
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

  // Register the command handler to be called when a command is received
  // from the Structure platform.
  device.onCommand(&handleCommand);

  connect();
}

void buttonPressed() {
  Serial.println("Button Pressed!");

  // Structure uses a JSON protocol. Construct the simple state object.
  // { "button" : true }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["button"] = true;

  // Send the state to Structure.
  device.sendState(root);
}

int buttonState = 0;

void loop() {

  bool toReconnect = false;

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if(!device.connected()) {
    Serial.println("Disconnected from Structure");
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

  delay(100);
}
