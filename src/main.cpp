/** 
 *  QuestEyes Firmware Package
 *  Copyright (C) 2022 Steven Wheeler.
 *  Contact: steven@stevenwheeler.co.uk
 * 
 *  This program is proprietary software.
 *  Modification of this software for personal use only is not prohibited, but also not recommended.
 *  Unauthorized redistribution and commercial use of this software is strictly prohibited, regardless of any modifications.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 *  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
 *  OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 *  This firmware is designed to run on the QuestEye device to communicate in conjunction with the QuestEye server software.
 **/

#include "main.h"
#include "unitidentifier.h"
#include "versioninfo.h"

//setup servers
/** 
* PORTS
*   7579 device discovery port
*   7580 commands/stream socket
**/
//ws is for transmission of camera data and receiving of commands
//udp is for transmission of connection discover packets
AsyncUDP discoveryUDP;
WebSocketsServer communicationSocket = WebSocketsServer(7580);

//define variables that are required further in the program
uint8_t cam_num;
String localIP;
String connectedIP;
int last_frame_timing = 0;
int last_heartbeat_timing = 0;
int frame_failure_count = 0;
bool connected = false;
bool otaMode = false;
int updateLength = 0;
int totalUpdateLength = 0;

//function to handle different websocket scenarios, such as connecting, disconnecting, and errors.
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      if (num == cam_num) {
        connected = false;
        otaMode = false;
        Serial.printf("Client disconnected (%s)\n", connectedIP.c_str());
        Serial.println("Closed stream connection to client.");
      }
      break;
    case WStype_CONNECTED:
      connected = true;
      cam_num = num;
      communicationSocket.sendTXT(cam_num, "NAME " + ("QuestEyes-" + unit_identifier));
      communicationSocket.sendTXT(cam_num, "FIRMWARE_VER " + firmware_version);
      //set connectedIP to the address of the connected client
      connectedIP = communicationSocket.remoteIP(cam_num).toString().c_str();
      Serial.printf("Client connected (%s).\n", connectedIP.c_str());
      Serial.println("Client is nominal.");
      break;
    case WStype_TEXT:
      Serial.printf("Command received: %s\n", payload);
      if (String((char*)payload).startsWith("OTA_MODE")) {
          //put the device into OTA mode
          Serial.println("Device entering OTA mode.");
          otaMode = true;
          communicationSocket.sendTXT(cam_num, "OTA_MODE_ACTIVE");
      };
      break;
    case WStype_BIN:
      if (otaMode == true) {
        //if the device is in OTA mode, then send the binary data to the OTA handler
        updateLength = 0;
        totalUpdateLength = 0;
        Serial.println("OTA update received, verifying...");
        //verify the payload is an official image that is not corrupt
        
        //if the payload is an image, begin the update
        if (String((char*)payload).startsWith("QE_UPDATE_IMG")) {
          //get the length of the image
          totalUpdateLength = String((char*)payload).substring(16).toInt();
          Serial.printf("Image length: %d\n", totalUpdateLength);
          //split the payload into chunks of 512 bytes in a byte array
          
          //send each chunk to the otaUpdateHandler until all chunks have been done
        }
      }
      break;
    case WStype_ERROR:      
      Serial.printf("Client experienced an error. Disconnecting...\n");
      communicationSocket.disconnect(num);
      Serial.printf("Disconnected client.\n");
      break;
    default:
      Serial.printf("Unknown websocket event.\n");
      break;
  }
}

//send chunks to this function to update, along with the size of the chunk.
void otaUpdateHandler(uint8_t *payload, size_t length) {
  Update.write(payload, length);
  updateLength += length;
  Serial.print('.');
  //send OTA update progress to the client as a percentage
  communicationSocket.sendTXT(cam_num, "OTA_UPDATE_PROGRESS " + String(updateLength * 100 / totalUpdateLength));
  if(updateLength != totalUpdateLength) return;
  Update.end(true);
  communicationSocket.sendTXT(cam_num, "OTA_UPDATE_COMPLETE");
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", updateLength);
  // Restart ESP32 to see changes 
  ESP.restart();
}

//method to capture a frame from the camera and transmit it to the client
void liveCam(uint8_t num) {
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    frame_failure_count++;
    //if the frame capture fails 30 times in a row, reboot the device
    if (frame_failure_count > 30) {
      Serial.println("Frame buffer could not be acquired for too long!");
      communicationSocket.sendTXT(num, "EXCESSIVE_FRAME_FAILURE");
      Serial.println("Device will now reboot due to frame buffer failure...");
      ESP.restart();
    }
      Serial.println("Frame buffer could not be acquired.");
      return;
  }
  frame_failure_count = 0;
  //send the frame to the client
  communicationSocket.sendBIN(num, fb->buf, fb->len);
  //release the frame buffer
  esp_camera_fb_return(fb);
}

// setup runs one time when reset is pressed or the board is powered
void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  //TODO: SETUP LED LIT WHITE WHILE BOOTING
  
  //prepare storage to check for wifi credentials
  Preferences storage;
  storage.begin("settings", true);

  //TODO: CHECK IF POWER BUTTON IS HELD LONG ENOUGH TO CLEAR CREDENTIALS AND RESET
  //TODO: HAVE IT SO THAT THE LEDS ALTERNATE WHITE AND ORANGE UNTIL RESET THRESHOLD
  //TODO: THEN ALTERNATE ORANGE AND GREEN WHILE RESETTING      
  //if power button is held, reset

  //if reset didnt happen, continue with setup
  //get the unique identifier from storage
  String uid = storage.getString("uid");
  //get wifi ssid and password from storage
  String ssid = storage.getString("ssid");
  String password = storage.getString("password");
  storage.end();
  //if uid is empty, populate it from the unitidentifier
  if(uid.length() == 0){
    storage.begin("settings", false);
    storage.putString("uid", unit_identifier);
    storage.end();
    uid = unit_identifier;
  }
  //if ssid and password are not empty, connect to wifi
  //try to connect to wifi for 30 seconds
  if(ssid.length() != 0 && password.length() != 0){
    //if ssid contains +'s, replace for spaces
    ssid.replace("+", " ");
    Serial.println("Connecting to WiFi...");
    WiFi.setHostname(("QuestEyes-" + unit_identifier).c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setSleep(false);
    int timeout = 0;
    while(WiFi.status() != WL_CONNECTED && timeout < 30000){

      //TODO: SETUP LED BLINKING WHITE TRYING TO CONNECT

      delay(100);
      timeout += 100;
      //print a dot every 500ms
      if(timeout % 500 == 0){
        Serial.print(".");
      }
    }
    //if connected, print out the ip address and continue the bootup process
    if(WiFi.status() == WL_CONNECTED){
      localIP = WiFi.localIP().toString();
      Serial.println("WiFi connected successfully.");
      Serial.println("Local IP address: " + localIP);

      //TODO: SETUP LED BLINKING GREEN AFTER A SUCCESSFUL CONNECTION

      //activate the OTA system
      Serial.println("Initializing OTA system...");
      //TODO: REWRITE OTA SYSTEM

      //initialize the camera
      Serial.println("Initializing camera...");
      initializeCam();

      //initialize the websocket server for information and OTA
      Serial.println("Initializing command socket...");
      communicationSocket.begin();
      communicationSocket.onEvent(webSocketEvent);

      //TODO: SETUP LED LIT GREEN FOR 3 SECONDS AFTER A SUCCESSFUL CAMERA SYSTEM INIT

    }
    //if failed to connect, start remote setup.
    else {
      Serial.println("Failed to connect to WiFi after 30 seconds - starting remote setup...");

      //TODO: MAKE THE SETUP PAGE LOOK PRETTY
      //TODO: SETUP LED FLASHING ORANGE AFTER A FAILED CONNECTION

      remoteSetup("QuestEyes-" + uid);
    }
  }
  //if ssid and password are empty, start remote setup.
  else {
    Serial.println("No Wifi credentials found - starting remote setup...");

    //TODO: SETUP LED FLASHING ORANGE AFTER NO WIFI CREDENTIALS

    remoteSetup("QuestEyes-" + uid);
  }

  //TODO: SETUP LED TURNING OFF AFTER A SUCCESSFUL BOOTUP
  
  Serial.println("System is ready for use.");
}

// main loop
void loop() {
  communicationSocket.loop();
  //if not connected...
  if(connected == false){
    //broadcast a mulicast packet on the network every second
    String broadcastString = "QUESTEYE_REQ_CONN:" + ("QuestEyes-" + unit_identifier) + ":" + localIP;
    discoveryUDP.broadcastTo(broadcastString.c_str(), 7579);
    delay(1000);
  }
  //if connected...
  if(connected == true){
    //send a heart every 5 seconds. If the server does not receive one within 10 seconds, it will assume the connection is dead.
      if(millis() - last_heartbeat_timing > 5000){
        communicationSocket.sendTXT(cam_num, "HEARTBEAT");
        last_heartbeat_timing = millis();
      };
    if(otaMode == false){
      //send frame
      if(millis() - last_frame_timing > 33.33){
        liveCam(cam_num);
        last_frame_timing = millis();
      };
    }
    else {
      //if ota mode is on...

    }
  }
}