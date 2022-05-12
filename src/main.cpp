/** 
 *  QuestEyes Firmware Package
 *  2022 Steven Wheeler.
 *  Contact: steven@stevenwheeler.co.uk
 * 
 *  This program is licenced under the GNU General Public License version 3.
 *  You can view the licence at http://www.gnu.org/licenses/gpl-3.0.html
 * 
 *  This firmware is designed to run on a QuestEye's hardware device to communicate in conjunction with the QuestEyes server software.
 **/

#include "main.h"

//define variables that are required further in the program
uint8_t cam_num;
String connectedIP;
int last_frame_timing = 0;
int last_heartbeat_timing = 0;
bool connected = false;
bool otaMode = false;

// setup runs to connect to internet and prepare broadcast to server, or starts remote setup
void setup()
{
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  //prepare storage to check for wifi credentials
  Preferences storage;
  storage.begin("settings", true);

  //get the unique identifier from storage
  String uid = storage.getString("uid");
  //get wifi ssid and password from storage
  String ssid = storage.getString("ssid");
  String password = storage.getString("password");
  storage.end();
  //if uid is empty, populate it from the unitidentifier
  if (uid.length() == 0)
  {
    storage.begin("settings", false);
    storage.putString("uid", unit_identifier);
    storage.end();
    uid = unit_identifier;
  }

  if (ssid.length() != 0 && password.length() != 0)
  { //if wifi credentials exist, continue startup
    startup(uid, ssid, password);
  }
  else
  { //else, start remote setup
    Serial.println("No Wifi credentials found - starting remote setup...");
    remoteSetup("QuestEyes-" + uid);
  }
}

// main loop
void loop()
{
  communicationSocket.loop();
  //if not connected...
  if (connected == false)
  {
    //broadcast a mulicast packet on the network every second
    String broadcastString = "QUESTEYE_REQ_CONN:" + ("QuestEyes-" + unit_identifier) + ":" + WiFi.localIP().toString();
    discoveryUDP.broadcastTo(broadcastString.c_str(), 7579);
    delay(1000);
  }
  //if connected...
  if (connected == true)
  {
    //send a heart every 5 seconds. If the server does not receive one within 10 seconds, it will assume the connection is dead.
    if (millis() - last_heartbeat_timing > 5000)
    {
      communicationSocket.sendTXT(cam_num, "HEARTBEAT");
      last_heartbeat_timing = millis();
    };
    if (otaMode == false)
    {
      //send frame (30 times per second for 30fps)
      if (millis() - last_frame_timing > 33.33)
      {
        captureCam(cam_num);
        last_frame_timing = millis();
      };
    }
    //hardware will still end heartbeats but not camera information if otaMode == true.
  }
}

//function to handle different websocket scenarios, such as connecting, disconnecting, and errors.
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    if (num == cam_num)
    {
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
    if (String((char *)payload).startsWith("OTA_MODE"))
    {
      //put the device into OTA mode
      Serial.println("Device entering OTA mode.");
      otaMode = true;
      communicationSocket.sendTXT(cam_num, "OTA_MODE_ACTIVE"); //send a confirmation of ota mode to the client
    };
    break;
  case WStype_BIN:
    if (otaMode == true)
    {
      processOTA(payload, length);
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