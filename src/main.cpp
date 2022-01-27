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

#define PART_BOUNDARY "123456789000000000000987654321"

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

camera_config_t config;

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
}

void configureCam(){
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
    //TODO: SETUP SYSTEM DECLARING ITS UNIQUE NAME TO ROUTER INSTEAD OF "ESP Arduino"
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
      Serial.println("WiFi connected successfully.");
      Serial.println("IP address: " + String(WiFi.localIP()));

      //TODO: SETUP LED BLINKING GREEN AFTER A SUCCESSFUL CONNECTION

      //activate the OTA system
      Serial.println("Initializing OTA system...");
      startOTA("QuestEyes-" + uid);

      //prepare the camera
      Serial.println("Initializing camera...");
      //initialize the camera using esp_camera
      configureCam();
      esp_err_t err = esp_camera_init(&config);
      if (err != ESP_OK) {
        Serial.println("Camera init failed with error: " + err);
        Serial.println("Device will now reboot due to camera init failure.");
        ESP.restart();
      } 
      //set camera parameters
      sensor_t * s = esp_camera_sensor_get();
      s->set_dcw(s, 1);
      s->set_special_effect(s, 2);
      s->set_whitebal(s, 1);
      s->set_raw_gma(s, 1);
      s->set_lenc(s, 1);

      //start camera server
      Serial.println("Starting camera server...");
      startCameraServer();
      Serial.println("Camera server ready.");

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
  //call the OTA system
	ArduinoOTA.handle();

  //loop
  delay(100);
}