
#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <PubSubClient.h>
// #include <TimeLib.h>
// #include <NtpClientLib.h>

//wifi creadentials
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";

//MQTT broker
const char* mqtt_server = "192.168.2.200";
const char* user = "guy";
const char* passw = "kupelu9e";

//MQTT topics
// NEED TO BE CHANGED FOR EVERY BOARD FLASHED
const char* deviceName = "Stove";
const char* deviceTopic = "HomePi/Dvir/ESP/test";
const char* stateTopic = "HomePi/Dvir/ESP/test/State";
const char* availTopic = "HomePi/Dvir/ESP/test/Avail";

// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
// following topics maybe unusable
const char* groupTopic = "HomePi/Dvir/All";
const char* alertTopic = "HomePi/Dvir/Alerts";
const char* topicArry[]={deviceTopic,groupTopic, alertTopic};

char msg[150];
char timeStamp[50];
char bootTime[50];
char *ver="1.4";
bool lastRelState;
bool curRelState;
bool toggleState=false;
bool firstRun = true;


// GPIO setup
int buttonPin = 0;
int relPin = 12;
int ledPin = 13;
int extPin = 14;

WiFiClient espClient;
// PubSubClient client(espClient);
void setup_wifi() {
        delay(10);
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);

        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }
        WiFi.setAutoReconnect(true);
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
}

void setup() {
  setup_wifi();
}
void func0 (){
        Serial.println("0");
}
void func1(){
        Serial.println("1");
}

void loop()
{
        func0();
        delay(1000);
        func1();
        delay(1000);

}
