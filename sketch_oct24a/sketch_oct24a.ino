
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson

// ~~~~~~~~ Server Related ~~~~~~~~~~~~~~~
// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputState = "off";


// ~~~~~~~~ Additional Parameters ~~~~~~~~~
// Assign output variables to GPIO pins
char output[20] = "5";
char mqtt_server[40] = "192.168.3.200";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
        Serial.println("Should save config");
        shouldSaveConfig = true;
}

bool readJSON_inflash(StaticJsonDocument <json_Size> &doc, char *filename){
        if (SPIFFS.begin()) {
                if (SPIFFS.exists(filename)) {
                        File _f = SPIFFS.open(filename, "r");
                        if (_f) {
                                DeserializationError error = deserializeJson(doc, _f);
                                if (error) {
                                        Serial.println("Error writing JSON to file");
                                        return 0;
                                }
                                else{
                                        return 1;
                                }
                        }
                        else {
                                Serial.println("Error opening file");
                                return 0;
                        }
                }
                else{
                        Serial.println("File does not exist");
                        return 0;
                }
        }
        else {
                return 0;
        }
}
bool saveJSON_inflash(StaticJsonDocument <json_Size> &doc, char *filename){
        if (SPIFFS.begin()) {
                File _f = SPIFFS.open(filename, "w");
                serializeJson(doc, _f);
                return 1;
        }
        else {
                return 0;
        }
}
bool getValue_JSON(const char* key, char *retValue, char *filename){
        StaticJsonDocument<json_Size> doc;

        if(readJSON_inflash(doc, filename)==1) {
                if(doc.containsKey(key)) {
                        const char* value = doc[key];
                        sprintf(retValue,"%s",value);
                        return 1;
                }
                else {
                        return 0;
                }
        }
        else{
                return 0;
        }
}
bool setValue_JSON(const char* key, char *value, char *filename){
        StaticJsonDocument<json_Size> doc;
        readJSON_inflash(doc, filename);

        doc[key] = value;
        if( saveJSON_inflash(doc, filename)==1) {
                return 1;
        }

        else{
                return 0;
        }
}
bool print_JSON(char *filename){
        StaticJsonDocument <json_Size> doc;

        if(readJSON_inflash(doc, filename)==1) {
                serializeJsonPretty(doc, Serial);
                return 1;
        }
        else{
                return 0;
        }
}

void startWifiManager(){
        WiFiManagerParameter custom_output("output", "output Pin", output, 2);
        WiFiManagerParameter mqttServer("MQTTsever", "MQTT server IP", mqtt_server, 38);

        // WiFiManager
        // Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wifiManager;

        //set config save notify callback
        wifiManager.setSaveConfigCallback(saveConfigCallback);

        // set custom ip for portal
        //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

        //add all your parameters here
        wifiManager.addParameter(&custom_output);
        wifiManager.addParameter(&mqttServer);

        // Uncomment and run it once, if you want to erase all the stored information
        // wifiManager.resetSettings();

        //set minimu quality of signal so it ignores AP's under that quality
        //defaults to 8%
        //wifiManager.setMinimumSignalQuality();

        //sets timeout until configuration portal gets turned off
        //useful to make it all retry or go to sleep
        //in seconds
        //wifiManager.setTimeout(120);

        // fetches ssid and pass from eeprom and tries to connect
        // if it does not connect it starts an access point with the specified name
        // here  "AutoConnectAP"
        // and goes into a blocking loop awaiting configuration
        wifiManager.autoConnect("myGuy");
        // or use this for auto generated name ESP + ChipID
        //wifiManager.autoConnect();

        // if you get here you have connected to the WiFi
        Serial.println("Connected.");
        strcpy(output, custom_output.getValue());
        strcpy(mqtt_server, mqttServer.getValue());
}
void startGPIO(){
// Initialize the output variables as outputs
        pinMode(atoi(output), OUTPUT);
// Set outputs to LOW
        digitalWrite(atoi(output), LOW);;
}
void serverLooper(){
        WiFiClient client = server.available(); // Listen for incoming clients

        if (client) {                 // If a new client connects,
                Serial.println("New Client."); // print a message out in the serial port
                String currentLine = ""; // make a String to hold incoming data from the client
                while (client.connected()) { // loop while the client's connected
                        if (client.available()) { // if there's bytes to read from the client,
                                char c = client.read(); // read a byte, then
                                Serial.write(c); // print it out the serial monitor
                                header += c;
                                if (c == '\n') { // if the byte is a newline character
                                        // if the current line is blank, you got two newline characters in a row.
                                        // that's the end of the client HTTP request, so send a response:
                                        if (currentLine.length() == 0) {
                                                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                                // and a content-type so the client knows what's coming, then a blank line:
                                                client.println("HTTP/1.1 200 OK");
                                                client.println("Content-type:text/html");
                                                client.println("Connection: close");
                                                client.println();

                                                // turns the GPIOs on and off
                                                if (header.indexOf("GET /output/on") >= 0) {
                                                        Serial.println("Output on");
                                                        outputState = "on";
                                                        digitalWrite(atoi(output), HIGH);
                                                } else if (header.indexOf("GET /output/off") >= 0) {
                                                        Serial.println("Output off");
                                                        outputState = "off";
                                                        digitalWrite(atoi(output), LOW);
                                                }

                                                // Display the HTML web page
                                                client.println("<!DOCTYPE html><html>");
                                                client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                                                client.println("<link rel=\"icon\" href=\"data:,\">");
                                                // CSS to style the on/off buttons
                                                // Feel free to change the background-color and font-size attributes to fit your preferences
                                                client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                                                client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
                                                client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                                                client.println(".button2 {background-color: #77878A;}</style></head>");

                                                // Web Page Heading
                                                client.println("<body><h1>ESP8266 Web Server</h1>");

                                                // Display current state, and ON/OFF buttons for the defined GPIO
                                                client.println("<p>Output - State " + outputState + "</p>");
                                                // If the outputState is off, it displays the ON button
                                                if (outputState == "off") {
                                                        client.println("<p><a href=\"/output/on\"><button class=\"button\">ON</button></a></p>");
                                                } else {
                                                        client.println("<p><a href=\"/output/off\"><button class=\"button button2\">OFF</button></a></p>");
                                                }
                                                client.println("</body></html>");

                                                // The HTTP response ends with another blank line
                                                client.println();
                                                // Break out of the while loop
                                                break;
                                        } else { // if you got a newline, then clear currentLine
                                                currentLine = "";
                                        }
                                } else if (c != '\r') { // if you got anything else but a carriage return character,
                                        currentLine += c; // add it to the end of the currentLine
                                }
                        }
                }
                // Clear the header variable
                header = "";
                // Close the connection
                client.stop();
                Serial.println("Client disconnected.");
                Serial.println("");
        }
}

void setup() {
        Serial.begin(115200);
        // SPIFFS.format();

        if (getValue_JSON("output",output, config_filename)==0) {
                setValue_JSON("output",output, config_filename);
        }
        if(getValue_JSON("mqtt_server",mqtt_server, config_filename)==0) {
                setValue_JSON("mqtt_server",mqtt_server, config_filename);
        }

        startWifiManager();

        if (shouldSaveConfig) {
                setValue_JSON("output",output, config_filename);
                setValue_JSON("mqtt_server",mqtt_server, config_filename);
        }

        startGPIO();
        print_JSON(config_filename);
        server.begin();
}

void loop() {
        serverLooper();
        delay(100);

}
