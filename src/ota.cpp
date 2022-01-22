#include "main.h"

/** Flag if OTA is enabled */
boolean otaStarted = false;

/** Limit the progress output on serial */
unsigned int lastProgress = 0;

/**
 * Initialize OTA server
 * and start waiting for OTA requests
 */
void startOTA(String identifier)
{
    ArduinoOTA
        .onStart([]() {
            String type;
            
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "flash";
            else
                type = "filesystem";

            //stop the RTSP stream to prepare for OTA
            stopRTSP();
            Serial.println("OTA start updating: " + type);
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

        //enable MDNS so device can be seen
	    ArduinoOTA.setMdnsEnabled(false);
	    String hostName = identifier;
        Serial.printf("Device is advertising as %s\n", hostName.c_str());
        //set the MDNS advertising name
        ArduinoOTA.setHostname(hostName.c_str());
        //start the OTA system
        ArduinoOTA.begin();
        Serial.println("OTA system ready to receive.");
    return;
}

void stopOTA(void)
{
	ArduinoOTA.end();
}