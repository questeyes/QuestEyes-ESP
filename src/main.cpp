#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "remotesetup.h"

const char* setup_ap_ssid = "QuestEyes-Setup-000000";
const char* setup_ap_pass = "questeyes";

// setup runs one time when reset is pressed or the board is powered
void setup() {
  declarePins();
  Preferences storage;
  storage.begin("settings", false);
  //get wifi ssid and password from storage
  String ssid = storage.getString("ssid");
  String password = storage.getString("password");
  storage.end();
  //if ssid and password are not empty, connect to wifi
  //try to connect to wifi for 30 seconds
  if(ssid != "" && password != ""){
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str());
    int timeout = 0;
    while(WiFi.status() != WL_CONNECTED && timeout < 30000){
      //TODO: SETUP LED FLASHING PULSING WHILE TRYING TO CONNECT
      delay(100);
      timeout += 100;
    }
    //if connected, print out the ip address and continue the bootup process
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("WiFi connected successfully.");
      Serial.println("IP address: " + String(WiFi.localIP()));
      //TODO: SETUP LED GOING GREEN AFTER A SUCCESSFUL CONNECTION
      return;
    }
    //if failed to connect, start remote setup.
    else {
      Serial.println("Failed to connect to WiFi after 30 seconds - starting remote setup...");
      //TODO: SETUP LED FLASHING ORANGE AFTER A FAILED CONNECTION
      remoteSetup();
    }
  }
  //if ssid and password are empty, start remote setup.
  else {
    Serial.println("No Wifi credentials found - starting remote setup...");
    //TODO: SETUP LED FLASHING ORANGE
    remoteSetup();
  }
}

// declare pins on the board
void declarePins() {

}

// main loop
void loop() {
  
}