//system files
#include <Arduino.h>
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems

//storage
#include <Preferences.h>

//wifi and ota
#include <WiFi.h>

//web sockets and communication
#include <WebSocketsServer.h>
#include <AsyncUDP.h>

//camera
#include "esp_camera.h"
#include "camera_pins.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"

//remote setup
void remoteSetup(String setup_ssid);