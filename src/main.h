//system files
#include <Arduino.h>
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems

//storage
#include <Preferences.h>

//wifi and ota
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "esp_https_server.h"
void startOTA(String identifier);

//web sockets and communication
#include <WebSocketsServer.h>

//camera
#include "esp_camera.h"
#include "camera_pins.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
void startCameraServer();

//remote setup
void remoteSetup(String setup_ssid);
