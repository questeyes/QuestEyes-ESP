#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "remotesetup.h"

const char* setup_ap_ssid = "QuestEyes-Setup-000000";
const char* setup_ap_pass = "questeyes";

// declare pins on the board
void declarePins() {

  //TODO: DECLARE PINS
  
}

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

      //activate the OTA system
      Serial.println("Initializing OTA system...");
      ArduinoOTA
        .onStart([]() {
            String type;
            
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else
                type = "filesystem";

            Serial.println("OTA start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nOTA finished. System rebooting...");
            ESP.restart();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("OTA authentication Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("OTA failed to start");
            else if (error == OTA_CONNECT_ERROR) Serial.println("OTA failed to connect");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA failed to recieve");
            else if (error == OTA_END_ERROR) Serial.println("OTA failed to end");
        });
        ArduinoOTA.begin();
        Serial.println("OTA system ready to receive.");

      //TODO: SETUP CAMERA SYSTEM READY TO STREAM

      return;
    }
    //if failed to connect, start remote setup.
    else {
      Serial.println("Failed to connect to WiFi after 30 seconds - starting remote setup...");

      //TODO: SETUP LED FLASHING ORANGE AFTER A FAILED CONNECTION

      remoteSetup(setup_ap_ssid, setup_ap_pass);
    }
  }
  //if ssid and password are empty, start remote setup.
  else {
    Serial.println("No Wifi credentials found - starting remote setup...");

    //TODO: SETUP LED FLASHING ORANGE

    remoteSetup(setup_ap_ssid, setup_ap_pass);
  }
}

// main loop
void loop() {
  //call the OTA system
  ArduinoOTA.handle();

  //TODO: SETUP CAMERA STREAM TO SERVER
  
}