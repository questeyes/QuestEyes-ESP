#include "main.h"

void remoteSetup(String setup_ssid) {
    //prep the IP address
    IPAddress local_ip(192,168,1,1);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);
    //setup web server
    WiFiServer server(80);
    //start the wifi hotspot for setup
    WiFi.softAP(setup_ssid.c_str(), NULL);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    Serial.println("AP serving on: " + local_ip);
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
            Serial.println("Client IP address: " + client.remoteIP());
            while (client.connected()) {
                //read the client's request
                String requestType = client.readStringUntil('\r');
                String fullRequest = client.readString();
                Serial.println(requestType);
                Serial.println(fullRequest);
                //if the request is not empty
                if(requestType != ""){
                    //if the request is a GET request for the index page
                    if(requestType.indexOf("GET / HTTP/1.1") != -1){
                        Serial.println("Serving setup page...");
                        //send the index page
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
                        client.println("<br>");
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
                        delay(500);
                        client.stop();
                    }
                    //if request is a POST request from the form
                    else if(requestType.indexOf("POST /wifisave") != -1){
                        Serial.println("Form was submitted. Saving wifi credentials...");
                        //get the ssid and password from the form
                        String ssid = fullRequest.substring(fullRequest.indexOf("ssid=") + 5, fullRequest.indexOf("&password="));
                        String password = fullRequest.substring(fullRequest.indexOf("password=") + 9, fullRequest.indexOf(" HTTP"));
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
                        client.println("<p>If your device cannot connect after 30 seconds, it will automatically restart this process so that you can reconnect and reinput your details.</p>");
                        client.println("<p>After a successful connection, the LED on your device with show up in green.</p>");
                        client.println("</body>");
                        client.println("</html>");
                        //confirm sent
                        delay(500);
                        Serial.println("Served.");
                        client.stop();
                        //reboot the system
                        Serial.println("System will now reboot to test new credentials...");
                        ESP.restart();
                    }
                    else {
                        Serial.println("Unknown request. Sending 404...");
                        client.println("HTTP/1.1 404 Not Found");
                        Serial.println("Served.");
                        delay(500);
                        client.stop();
                    }
                }
            }
        }
    }
}