#include "main.h"
#include "unitidentifier.h"

OV2640 cam;

// declare pins on the board
void declarePins() {

}

// setup runs one time when reset is pressed or the board is powered
void setup() {
  declarePins();
  //prepare storage to check for wifi credentials

  //TODO: SETUP LED WHITE WHILE BOOTING

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

      //TODO: SETUP LED FLASHING PULSING WHITE WHILE TRYING TO CONNECT

      delay(100);
      timeout += 100;
      //print a dot every 500ms
      if(timeout % 500 == 0){
        Serial.print(".");
      }
    }
    //if connected, print out the ip address and continue the bootup process
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("WiFi connected successfully.");
      Serial.println("IP address: " + String(WiFi.localIP()));

      //TODO: SETUP LED FLASHING WHITE AND GREEN AFTER A SUCCESSFUL CONNECTION

      //activate the OTA system
      Serial.println("Initializing OTA system...");
      startOTA("QuestEyes-" + unit_identifier);

      //prepare the camera
      delay(100);
      cam.init(esp32cam_aithinker_config);
      delay(100);
      Serial.println("Initializing camera...");
      Serial.print("RTSP server: rtsp://");
      Serial.print(WiFi.localIP());
      Serial.println(":8554/mjpeg/1\n");
	    initRTSP();

      //TODO: SETUP LED FLASHING GREEN FOR 2 SECONDS AFTER A SUCCESSFUL CAMERA SYSTEM INIT

    }
    //if failed to connect, start remote setup.
    else {
      Serial.println("Failed to connect to WiFi after 30 seconds - starting remote setup...");

      //TODO: SETUP LED FLASHING ORANGE AFTER A FAILED CONNECTION

      remoteSetup("QuestEyes-" + unit_identifier);
    }
  }
  //if ssid and password are empty, start remote setup.
  else {
    Serial.println("No Wifi credentials found - starting remote setup...");

    //TODO: SETUP LED FLASHING ORANGE AFTER NO WIFI CREDENTIALS

    remoteSetup("QuestEyes-" + unit_identifier);
  }

  //TODO: SETUP LED TURNING OFF AFTER A SUCCESSFUL BOOTUP
  
  Serial.println("System is ready for use.");
}

// main loop
void loop() {
  //call the OTA system
	ArduinoOTA.handle();

  //loop
  delay(100);
}