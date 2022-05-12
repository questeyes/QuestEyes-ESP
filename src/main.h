//system files
#include <Arduino.h>
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems

//hardware information
#include "unitidentifier.h"
#include "versioninfo.h"

//storage
#include <Preferences.h>

//wifi and ota
#include <WiFi.h>
#include <ArduinoOTA.h>
void processOTA(uint8_t *payload, int length);

//web sockets and communication
#include <WebSocketsServer.h>
#include <AsyncUDP.h>
/** 
* PORTS
*   7579 device discovery port
*   7580 commands/stream socket
**/
//udp is for transmission of connection discovery packets
//ws is for transmission of camera data, recieving of commands and OTA files for updates
AsyncUDP discoveryUDP;
WebSocketsServer communicationSocket = WebSocketsServer(7580);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

//camera
#include "esp_camera.h"
#include "camera_pins.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
void initializeCam();
void captureCam(uint8_t num);

//remote setup
void remoteSetup(String setup_ssid);

//startup
void startup(String uid, String ssid, String password);