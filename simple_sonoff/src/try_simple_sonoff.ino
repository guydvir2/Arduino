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
const char* deviceName = "Sonoff1";
const char* deviceTopic = "HomePi/Dvir/Switches/S1";
const char* msgTopic = "HomePi/Dvir/Messages";
const char* groupTopic = "HomePi/Dvir/All";
const char* alertTopic = "HomePi/Dvir/Alerts";
const char* topicArry[]={deviceTopic,groupTopic, alertTopic};

char msg[150];
char timeStamp[50];
char bootTime[50];
char* ver="1.2";
bool lastRelState;
bool curRelState;
bool toggleState=false;


// GPIO setup
int buttonPin = 0;
int relPin = 12;
int ledPin = 13;
int extPin = 14;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
        Serial.begin(9600);

        pinMode(buttonPin, INPUT);
        pinMode(extPin, INPUT_PULLUP);
        pinMode(relPin, OUTPUT);
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin,HIGH); // means OFF
        digitalWrite(relPin, !digitalRead(ledPin));

        setup_wifi();
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000*3600*4); // 4hrs
        get_timeStamp();
        strcpy(bootTime,timeStamp);

        ledBlink(100,4);
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

void ledBlink(int delay_time, int amount) {
        bool ledState = LOW;
        for (int i = 0; i < amount; i++) {
                digitalWrite(ledPin, ledState);
                delay(delay_time);
                ledState = !ledState;
                delay(delay_time);
        }
}

void callback(char* topic, byte* payload, unsigned int length) {
        char incoming_msg[50];

        // ledBlink(30, 2);
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length]=0;
        Serial.println("");

        //  comply with action
        curRelState = digitalRead(relPin);
        if (strcmp(incoming_msg,"status")==0) {
                sprintf(msg,"Status CMD: Relay State: [%d], bootTime: [%s], [v%s]",curRelState,bootTime,ver);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg,"on")==0) {
                digitalWrite(relPin, HIGH);
                sprintf(msg,"Topic: [%s] Remote CMD: [ON]",topic);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg,"off")==0) {
                digitalWrite(relPin, LOW);
                sprintf(msg,"Topic: [%s] Remote CMD: [OFF]",topic);
                pub_msg(msg);
        }
}

void reconnect() {
        // Loop until we're reconnected
        while (!client.connected()) {
                Serial.print("Attempting MQTT connection...");
                // Attempt to connect
                if (client.connect(deviceName,user, passw)) {
                        Serial.println("connected");
                        pub_msg("Connected to MQTT server");

                        for (int i=0; i<sizeof(topicArry)/sizeof(char *); i++) {
                                client.subscribe(topicArry[i]);
                                sprintf(msg, "Subscribed to %s",topicArry[i]);
                                pub_msg(msg);
                        }
                } else {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 5 seconds");
                        delay(5000);
                }
        }
}

void pub_msg(char *inmsg){
        char tmpmsg[200];
        get_timeStamp();
        sprintf(tmpmsg,"[%s] [%s] %s",timeStamp,deviceTopic, inmsg);
        client.publish(msgTopic, tmpmsg);
}

void get_timeStamp(){
        time_t t=now();
        sprintf(timeStamp,"%02d-%02d-%d %02d:%02d:%02d:00",year(t),month(t), day(t), hour(t), minute(t), second(t));
}

void loop() {
        if (!client.connected()) {
                reconnect();
        }
        client.loop();

        // corrolate Relstate to LedState
        curRelState=digitalRead(relPin);
        digitalWrite(ledPin,!curRelState);

        // EXT released (GPIO14)
        if (digitalRead(extPin)==HIGH) {
                delay(50);
                if (digitalRead(extPin)==HIGH && lastRelState!=LOW) {
                        digitalWrite(relPin,HIGH);
                        lastRelState=LOW;
                        pub_msg("Ext.Button pressed [ON]");
                }
        }
        // EXT pressed (GPIO14)
        else if (digitalRead(extPin)==LOW) {
                delay(50);
                if (digitalRead(extPin)==LOW && lastRelState!=HIGH) {
                        digitalWrite(relPin,LOW);
                        lastRelState=HIGH;
                        pub_msg("Ext.Button pressed [OFF]");

                }
        }
        // Button pressed ( toggle on/off)
        if (digitalRead(buttonPin)==0) {
                toggleState = !toggleState;
                digitalWrite(relPin, toggleState);
                sprintf(msg, "Button toggled [%s]",toggleState ? "ON" : "OFF");
                pub_msg(msg);
                delay(300);
        }

        delay(50);
}
