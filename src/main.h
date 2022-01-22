//ardunio file
#include <Arduino.h>

//storage
#include <Preferences.h>

//wifi and ota
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
void startOTA(String identifier);
void stopOTA(void);

//camera
#include "OV2640.h"
#include "OV2640Streamer.h"
#include "CRtspSession.h"

//remote setup
void remoteSetup(String setup_ssid);

//camera server
#define ENABLE_RTSPSERVER
extern OV2640 cam;
void initRTSP(void);
void stopRTSP(void);
