#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

//wifi creadentials
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";

//MQTT broker
const char* mqtt_server = "192.168.2.200";
const char* user = "guy";
const char* passw = "kupelu9e";

//MQTT topics
// NEED TO BE CHANGED FOR EVERY BOARD FLASHED
const char* deviceName = "windows_test1";
const char* deviceTopic = "HomePi/Dvir/Windows/test1";
const char* stateTopic = "HomePi/Dvir/Windows/test1/State";
const char* availTopic = "HomePi/Dvir/Windows/test1/Avail";

// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
// following topics maybe unusable
const char* groupTopic = "HomePi/Dvir/All";
const char* alertTopic = "HomePi/Dvir/Alerts";
const char* topicArry[]={deviceTopic,groupTopic, alertTopic};

char msg[150];
char timeStamp[50];
char bootTime[50];
const char* ver="2.0";
bool lastRel_1_State;
bool curRel_1_State;
bool toggleState_0=false;
bool toggleState_1=false;
bool firstRun = true;


// GPIO setup
const int extButton0 = 0;
const int extButton1 = 9;
const int relPin0 = 12;
const int relPin1 = 5;
const int ledPin0 = 13;
const int swLocal = 10;
bool temp;


WiFiClient espClient;
PubSubClient client(espClient);

void ledBlink(int delay_time, int amount) {
        bool ledState = LOW;
        for (int i = 0; i < amount; i++) {
                digitalWrite(ledPin0, ledState);
                delay(delay_time);
                ledState = !ledState;
                delay(delay_time);
        }
}

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
        Serial.begin(9600);
        pinMode(swLocal, INPUT);
        pinMode(ledPin0, OUTPUT);
        pinMode(extButton0, INPUT_PULLUP);
        pinMode(relPin0, OUTPUT);
        pinMode(extButton1, INPUT_PULLUP);
        pinMode(relPin1, OUTPUT);

        // ON on boot
        digitalWrite(ledPin0,LOW); // means OFF
        toggleState_0 = true;
        toggleState_1 = true;
        // in case of change -- > there is a retain msg to change !

        setup_wifi();
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000*3600*4); // 4hrs
        // get_timeStamp();
        // strcpy(bootTime,timeStamp);

        ledBlink(100,4);
}

// void callback(char* topic, byte* payload, unsigned int length) {
//         char incoming_msg[50];
//
//         // ledBlink(30, 2);
//         Serial.print("Message arrived [");
//         Serial.print(topic);
//         Serial.print("] ");
//         for (int i = 0; i < length; i++) {
//                 Serial.print((char)payload[i]);
//                 incoming_msg[i] = (char)payload[i];
//         }
//         incoming_msg[length]=0;
//         Serial.println("");
//
//         //  comply with action
//         curRel_1_State = digitalRead(relPin_1);
//         if (strcmp(incoming_msg,"status")==0) {
//                 sprintf(msg,"Status CMD: Relay State: [%d], bootTime: [%s], [v%s]",curRel_1_State,bootTime,ver);
//                 pub_msg(msg);
//         }
//         else if (strcmp(incoming_msg,"on")==0) {
//                 digitalWrite(relPin_1, HIGH);
//                 sprintf(msg,"Topic: [%s] Remote CMD: [ON]",topic);
//                 pub_msg(msg);
//         }
//         else if (strcmp(incoming_msg,"off")==0) {
//                 digitalWrite(relPin_1, LOW);
//                 sprintf(msg,"Topic: [%s] Remote CMD: [OFF]",topic);
//                 pub_msg(msg);
//         }
// }
//
// void reconnect() {
//         // Loop until we're reconnected
//         while (!client.connected()) {
//                 Serial.print("Attempting MQTT connection...");
//                 // Attempt to connect
//                 if (client.connect(deviceName,user, passw, availTopic,0,true,"offline")) {
//                         Serial.println("connected");
//                         client.publish(availTopic, "online", true);
//                         if (firstRun == true) {
//                           client.publish(stateTopic, "on", true);
//                           firstRun = false;
//                         }
//                         pub_msg("Connected to MQTT server");
//
//                         for (int i=0; i<sizeof(topicArry)/sizeof(char *); i++) {
//                                 client.subscribe(topicArry[i]);
//                                 sprintf(msg, "Subscribed to %s",topicArry[i]);
//                         }
//                 } else {
//                         Serial.print("failed, rc=");
//                         Serial.print(client.state());
//                         Serial.println(" try again in 5 seconds");
//                         delay(5000);
//                 }
//         }
// }
//
// void pub_msg(char *inmsg){
//         char tmpmsg[200];
//         get_timeStamp();
//         sprintf(tmpmsg,"[%s] [%s] %s",timeStamp,deviceTopic, inmsg);
//         client.publish(msgTopic, tmpmsg);
// }
//
// void get_timeStamp(){
//         time_t t=now();
//         sprintf(timeStamp,"%02d-%02d-%d %02d:%02d:%02d:00",year(t),month(t), day(t), hour(t), minute(t), second(t));
// }
//
// void loop() {
//         if (!client.connected()) {
//                 reconnect();
//         }
//         client.loop();
//
//         // corrolate Relstate to LedState
//         curRel_1_State=digitalRead(relPin_1);
//         digitalWrite(ledPin_1,!curRel_1_State);
//
//         // EXT released (GPIO14)
//         if (digitalRead(extPin_1)==HIGH) {
//                 delay(50);
//                 if (digitalRead(extPin_1)==HIGH && lastRel_1_State!=LOW) {
//                         digitalWrite(relPin_1,HIGH);
//                         lastRel_1_State=LOW;
//                         client.publish(stateTopic, "on", true);
//                         pub_msg("Ext.Button pressed [ON]");
//                 }
//         }
//         // EXT pressed (GPIO14)
//         else if (digitalRead(extPin_1)==LOW) {
//                 delay(50);
//                 if (digitalRead(extPin_1)==LOW && lastRel_1_State!=HIGH) {
//                         digitalWrite(relPin_1,LOW);
//                         lastRel_1_State=HIGH;
//                         client.publish(stateTopic, "off", true);
//                         pub_msg("Ext.Button pressed [OFF]");
//                 }
//         }
//         // Button pressed ( toggle on/off)
//         if (digitalRead(buttonPin)==0) {
//                 toggleState_0 = !toggleState_0;
//                 digitalWrite(relPin_1, toggleState_0);
//                 sprintf(msg, "Button toggled [%s]",toggleState_0 ? "ON" : "OFF");
//                 pub_msg(msg);
//                 sprintf(msg, "%s",toggleState_0 ? "on" : "off");
//                 client.publish(stateTopic, msg, true);
//                 delay(300);
//         }
//
//         delay(50);
// }
