/** 
 *  QuestEyes Firmware Package
 *  Copyright (C) 2022 Steven Wheeler.
 *  Contact: steven@stevenwheeler.co.uk
 * 
 *  This program is proprietary software. There is NO public license.
 *  Unauthorized redistribution of this software is strictly prohibited.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 *  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
 *  OR OTHER DEALINGS IN THE SOFTWARE.
 **/

#include "main.h"
#include "unitidentifier.h"
#include "versioninfo.h"

WebSocketsServer ws = WebSocketsServer(7580);
AsyncUDP udp;

uint8_t cam_num;
String IP;
int last_heartbeat = 0;
bool connected = false;
int frame_failure_count = 0;

void initializeCam(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error: " + err);
    Serial.println("Device will now reboot due to camera init failure...");    
    ESP.restart();
    return;
  }
}

void liveCam(uint8_t num){
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    frame_failure_count++;
    if (frame_failure_count > 30) {
      Serial.println("Frame buffer could not be acquired for too long!");
      ws.sendTXT(cam_num, "EXCESSIVE_FRAME_FAILURE");
      Serial.println("Device will now reboot due to frame buffer failure...");
      ESP.restart();
    }
      Serial.println("Frame buffer could not be acquired.");
      return;
  }
  frame_failure_count = 0;
  ws.sendBIN(num, fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("Client disconnected.\n");
      connected = false;
      break;
    case WStype_CONNECTED:
      connected = true;
      if (num > 0)  {
        ws.disconnect(num);
      }
      cam_num = num;
      ws.sendTXT(cam_num, "NAME " + ("QuestEyes-" + unit_identifier));
      ws.sendTXT(cam_num, "FIRMWARE_VER " + firmware_version);
      Serial.printf("Client connected.\n");
      break;
    case WStype_TEXT:
    case WStype_BIN:
    case WStype_ERROR:      
      Serial.printf("Client experienced an error. Disconnecting...\n");
      ws.disconnect(num);
      connected = false;
      Serial.printf("Disconnected client.\n");
      break;
  }
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
      IP = WiFi.localIP().toString();
      Serial.println("WiFi connected successfully.");
      Serial.println("IP address: " + IP);

      //TODO: SETUP LED BLINKING GREEN AFTER A SUCCESSFUL CONNECTION

      //activate the OTA system
      Serial.println("Initializing OTA system...");
      //TODO: REWRITE OTA SYSTEM

      //initialize the camera
      Serial.println("Initializing camera...");
      initializeCam();

      //initialize the websocket server for information and OTA
      Serial.println("Initializing transceive socket...");
      ws.begin();
      ws.onEvent(webSocketEvent);

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
  ws.loop();
  //if not connected, broadcast a packet on the network
  if(connected == false){
    //send a mulicast packet every second
    String broadcastString = "QUESTEYE_REQ_CONN:" + ("QuestEyes-" + unit_identifier) + ":" + IP;
    udp.broadcastTo(broadcastString.c_str(), 7579);
    delay(1000);
  }
  if(connected == true){
    //send a heartbeat every 10 seconds
    if(millis() - last_heartbeat > 5000){
      ws.sendTXT(cam_num, "HEARTBEAT");
      last_heartbeat = millis();
    }
    //send frame
    liveCam(cam_num);
  }
}