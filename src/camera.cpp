#include "main.h"

//initialize the camera using this function
void initializeCam(){
  //start a config
  camera_config_t config;
  //populate the config with the correct information for the camera
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
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  //initialize the camera with the config
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error: " + err);
    Serial.println("Device will now reboot due to camera init failure...");    
    ESP.restart();
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_hmirror(s, 1);
  s->set_dcw(s, 1);

  s->set_res_raw(s, 0, 0, 0, 0, 0, 244, 1536, 732, 768, 300, true, true);
}

int frame_failure_count = 0;
void captureCam(uint8_t num)
{
  //capture a frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    frame_failure_count++;
    //if the frame capture fails 30 times in a row, reboot the device
    if (frame_failure_count > 30)
    {
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