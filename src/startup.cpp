#include "main.h"

void startup(String uid, String ssid, String password)
{
    //TODO: SETUP LED LIT WHITE WHILE BOOTING

    //TODO: CHECK IF POWER BUTTON IS HELD LONG ENOUGH TO CLEAR CREDENTIALS AND RESET
    //TODO: HAVE IT SO THAT THE LEDS ALTERNATE WHITE AND ORANGE UNTIL RESET THRESHOLD
    //TODO: THEN ALTERNATE ORANGE AND GREEN WHILE RESETTING

    //initialize the camera
    Serial.println("Initializing hardware (camera assembly)...");
    initializeCam();

    //try to connect to wifi for 30 seconds
    //if ssid contains +'s, replace for spaces
    ssid.replace("+", " ");
    Serial.println("Connecting to WiFi...");
    WiFi.setHostname(("QuestEyes-" + uid).c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setSleep(false);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 30000)
    {

        //TODO: SETUP LED BLINKING WHITE TRYING TO CONNECT

        delay(100);
        timeout += 100;
        //print a dot every 500ms
        if (timeout % 500 == 0)
        {
            Serial.print(".");
        }
    }
    //if connected, print out the ip address and continue the bootup process
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected successfully.");
        Serial.println("Local IP address: " + WiFi.localIP().toString());

        //TODO: SETUP LED BLINKING GREEN AFTER A SUCCESSFUL CONNECTION

        //initialize the websocket server for information and OTA
        Serial.println("Initializing command socket...");
        communicationSocket.begin();
        communicationSocket.onEvent(webSocketEvent);

        //TODO: SETUP LED LIT GREEN FOR 3 SECONDS AFTER A SUCCESSFUL CAMERA SYSTEM INIT
    }
    //if failed to connect, start remote setup.
    else
    {
        Serial.println("Failed to connect to WiFi after 30 seconds - starting remote setup...");

        //TODO: SETUP LED FLASHING ORANGE AFTER A FAILED CONNECTION

        remoteSetup("QuestEyes-" + uid);
    }

    //TODO: SETUP LED TURNING OFF AFTER A SUCCESSFUL BOOTUP

    Serial.println("System is ready for use.");
}