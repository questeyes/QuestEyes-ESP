#include "remotesetup.h"

const char* setup_ap_ssid = "QuestEyes-Setup-000000";
const char* setup_ap_pass = "questeyes";

int remoteSetup(){
    //setup web server
    WiFiServer server(80);
    //start the wifi hotspot for setup
    WiFi.softAP(setup_ap_ssid, setup_ap_pass);
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("AP IP address: " + String(myIP));
    //start the web server
    server.begin();
    //start listening for clients
    Serial.println("Waiting for client...");
    while(true){
        //check if a client has connected
        WiFiClient client = server.available();
        if(client){
            //if a client has connected, print out the client's ip address
            Serial.println("Client connected.");
            Serial.println("Client IP address: " + String(client.remoteIP()));
            while (client.connected()) {
                //read the client's request
                String request = client.readStringUntil('\r');
                Serial.println(request);
                //if the request is not empty
                if(request != ""){
                    //if the request is a GET request
                    if(request.indexOf("GET") != -1){
                        //if the request is for the index page
                        Serial.println("Serving setup page...");
                        if(request.indexOf("/index.html") != -1){
                            //send the setup page
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-Type: text/html");
                            client.println("");
                            client.println("<!DOCTYPE HTML>");
                            client.println("<html>");
                            client.println("<head>");
                            client.println("<title>QuestEyes Setup</title>");
                            client.println("</head>");
                            client.println("<body>");
                            client.println("<h1>QuestEyes Setup</h1>");
                            client.println("<p>Please enter the wifi credentials you want your QuestEyes to use below.</p>");
                            client.println("<form action=\"/wifisave\" method=\"post\">");
                            client.println("<label for=\"ssid\">SSID:</label>");
                            client.println("<input type=\"text\" name=\"ssid\" id=\"ssid\" value=\"\" />");
                            client.println("<label for=\"password\">Password:</label>");
                            client.println("<input type=\"password\" name=\"password\" id=\"password\" value=\"\" />");
                            client.println("<input type=\"submit\" value=\"Submit\" />");
                            client.println("</form>");
                            client.println("</body>");
                            client.println("</html>");
                            //confirm sent
                            Serial.println("Served.");
                        }
                    }
                    //if request is a POST request from the form
                    else if(request.indexOf("POST /wifisave") != -1){
                        Serial.println("Form was submitted. Saving wifi credentials...");
                        //get the ssid and password from the form
                        String ssid = request.substring(request.indexOf("ssid=") + 5, request.indexOf("&password="));
                        String password = request.substring(request.indexOf("password=") + 9, request.indexOf(" HTTP"));
                        //save the ssid and password to storage
                        Preferences storage;
                        storage.begin("settings", false);
                        storage.putString("ssid", ssid);
                        storage.putString("password", password);
                        storage.end();
                        //confirm saved
                        Serial.println("Wifi credentials saved. Serving confirmation to user...");
                        //send the user a message that the wifi credentials were saved
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("");
                        client.println("<!DOCTYPE HTML>");
                        //send the user a message that the wifi credentials were saved
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>QuestEyes Setup</title>");
                        client.println("</head>");
                        client.println("<body>");
                        client.println("<h1>QuestEyes Setup</h1>");
                        client.println("<p>Wifi credentials saved to device.</p>");
                        client.println("<p>Your QuestEyes will now reboot and try to connect.</p>");
                        client.println("<p>If your device cannot connect after 30 seconds, it will automatically restart this process so you can reconnect and reinput your details.</p>");
                        client.println("<p>After a successful connection, the LED on your device with show up in green.</p>");
                        client.println("</body>");
                        client.println("</html>");
                        //confirm sent
                        Serial.println("Served.");
                        //reboot the system
                        Serial.println("System will now reboot to test new credentials...");
                        ESP.restart();
                    }
                }
            }
            //close the connection
            client.stop();
            Serial.println("Client disconnected.");
        }
    }
}