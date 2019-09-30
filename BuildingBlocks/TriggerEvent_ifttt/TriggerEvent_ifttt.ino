#include <IFTTTMaker.h>

/*******************************************************************
    Trigger event at end of the setup
*                                                                 *
    adapted by Brian Lough
*******************************************************************/


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//------- Replace the following! ------
char ssid[] = "Xiaomi_ADA6";       // your network SSID (name)
char password[] = "guyd5161";  // your network key



#define KEY "cFLymB4JT9tlODsKLFn9TA"  // Get it from this page https://ifttt.com/services/maker/settings
// #define EVENT_NAME "upload_data" // Name of your event name, set when you are creating the applet
#define EVENT_NAME "Notify"
WiFiClientSecure client;
IFTTTMaker ifttt(KEY, client);

bool upload_data(char *filename, char *cell_num, char *cell_value) {
        if (ifttt.triggerEvent(EVENT_NAME, filename, cell_num, cell_value)) {
                return 1;
        }
        else {
                return 0;
        }
}

void setup() {

        Serial.begin(115200);

        // Set WiFi to station mode and disconnect from an AP if it was Previously
        // connected
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        // Attempt to connect to Wifi network:
        Serial.print("Connecting Wifi: ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                delay(500);
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        IPAddress ip = WiFi.localIP();
        Serial.println(ip);

        client.setInsecure();

        //triggerEvent takes an Event Name and then you can optional pass in up to 3 extra Strings
        upload_data("test2","A1","KAKI");

}

void loop() {
}
