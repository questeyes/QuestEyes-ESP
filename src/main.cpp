#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "remotesetup.h"
#include "otaupdates.h"

const char* setup_ap_ssid = "QuestEyes-Setup-000000";
const char* setup_ap_pass = "questeyes";

// setup runs one time when reset is pressed or the board is powered
void setup() {
  declarePins();
  //prepare storage to check for wifi credentials
  Preferences storage;
  storage.begin("settings", false);

  //TODO: CHECK IF POWER BUTTON IS HELD LONG ENOUGH TO CLEAR CREDENTIALS AND RESET
  //TODO: HAVE IT SO THAT THE LEDS GRADUALLY LIGHT UNTIL RESET
  //if power button is held, reset

  //if reset didnt happen, continue with setup
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

      //TODO: SETUP CAMERA SYSTEM READY TO STREAM

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

  //TODO: DECLARE PINS
  
}

// main loop
void loop() {
  //call the OTA system
  listenForOTA();

  //TODO: SETUP CAMERA STREAM TO SERVER
  
}