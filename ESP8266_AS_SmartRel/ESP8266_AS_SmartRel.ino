//#include <Arduino.h>
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
const char* deviceName = "test1_esp";
const char* deviceTopic = "HomePi/Dvir/ESP/test";
const char* stateTopic = "HomePi/Dvir/ESP/test/State";
const char* availTopic = "HomePi/Dvir/ESP/Stest/Avail";

// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
// following topics maybe unusable
const char* groupTopic = "HomePi/Dvir/All";
const char* alertTopic = "HomePi/Dvir/Alerts";
const char* topicArry[]={deviceTopic,groupTopic, alertTopic};

char msg[150];
char timeStamp[50];
char bootTime[50];
bool Rel_0_state = false;
bool Rel_1_state = false;
bool lastSW_0_state = false;
bool lastSW_1_state = false;
//char* ver="1.4";
//bool lastRelState;
//bool curRelState;
//bool toggleState=false;
//bool firstRun = true;


// GPIO setup
//int buttonPin = 0;
const int Sw_0_Pin = 2;
const int Sw_1_Pin = 3;
const int Rel_0_Pin = 4;
const int Rel_1_Pin = 5;


//int extPin = 14;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
        Serial.begin(9600);

        pinMode(Sw_0_Pin, INPUT_PULLUP);
        pinMode(Sw_1_Pin, INPUT_PULLUP);
        pinMode(Rel_0_Pin, OUTPUT);
        pinMode(Rel_1_Pin, OUTPUT);

        digitalWrite(Rel_0_Pin,LOW);
        digitalWrite(Rel_1_Pin,LOW);


        // ON on boot
//        digitalWrite(ledPin,LOW); // means OFF
//        digitalWrite(relPin, HIGH);
//        toggleState = true;
//        // in case of change -- > there is a retain msg to change !

        setup_wifi();
        client.setServer(mqtt_server, 1883);
//        client.setCallback(callback);
//        NTP.begin("pool.ntp.org", 2, true);
//        NTP.setInterval(1000*3600*4); // 4hrs
//        get_timeStamp();
//        strcpy(bootTime,timeStamp);

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
//

//
//void callback(char* topic, byte* payload, unsigned int length) {
//        char incoming_msg[50];
//
//        // ledBlink(30, 2);
//        Serial.print("Message arrived [");
//        Serial.print(topic);
//        Serial.print("] ");
//        for (int i = 0; i < length; i++) {
//                Serial.print((char)payload[i]);
//                incoming_msg[i] = (char)payload[i];
//        }
//        incoming_msg[length]=0;
//        Serial.println("");
//
//        //  comply with action
//        curRelState = digitalRead(relPin);
//        if (strcmp(incoming_msg,"status")==0) {
//                sprintf(msg,"Status CMD: Relay State: [%d], bootTime: [%s], [v%s]",curRelState,bootTime,ver);
//                pub_msg(msg);
//        }
//        else if (strcmp(incoming_msg,"on")==0) {
//                digitalWrite(relPin, HIGH);
//                sprintf(msg,"Topic: [%s] Remote CMD: [ON]",topic);
//                pub_msg(msg);
//        }
//        else if (strcmp(incoming_msg,"off")==0) {
//                digitalWrite(relPin, LOW);
//                sprintf(msg,"Topic: [%s] Remote CMD: [OFF]",topic);
//                pub_msg(msg);
//        }
//}
//
void reconnect() {
        // Loop until we're reconnected
        while (!client.connected()) {
                Serial.print("Attempting MQTT connection...");
                // Attempt to connect
                if (client.connect(deviceName,user, passw, availTopic,0,true,"offline")) {
                        Serial.println("connected");
                        client.publish(availTopic, "online", true);
                        if (firstRun == true) {
                          client.publish(stateTopic, "on", true);
                          firstRun = false;
                        }
                        pub_msg("Connected to MQTT server");

                        for (int i=0; i<sizeof(topicArry)/sizeof(char *); i++) {
                                client.subscribe(topicArry[i]);
                                sprintf(msg, "Subscribed to %s",topicArry[i]);
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
        sprintf(timeStamp,"%02d-%02d-%02d %02d:%02d:%02d:00",year(t),month(t), day(t), hour(t), minute(t), second(t));
}

void switchUp(){
  digitalWrite(Rel_1_Pin, false);
  Rel_1_state=false;
  delay(50);
  digitalWrite(Rel_0_Pin, true);
  Rel_0_state=true;
}

void switchDown(){
  digitalWrite(Rel_0_Pin, false);
  Rel_0_state=false;
  delay(50);
  digitalWrite(Rel_1_Pin, true);
  Rel_1_state=true;
}

void switchOff(){
  digitalWrite(Rel_0_Pin, false);
  Rel_0_state=false;
  delay(50);
  digitalWrite(Rel_1_Pin, false);
  Rel_1_state=false;
}

void loop() {
  digitalWrite(Sw_0_Pin,HIGH);
  delay(100);
  digitalWrite(Sw_0_Pin,LOW);
  delay(100);

  bool rel_0 = digitalRead(Rel_0_Pin);
  bool rel_1 = digitalRead(Rel_1_Pin);

  if (digitalRead(Sw_0_Pin) == true && lastSW_0_state == false){
    switchUp();
  }

  else if (digitalRead(Sw_0_Pin) == false && lastSW_0_state == true){
    switchOff();
  }

  if (digitalRead(Sw_1_Pin) == true && lastSW_1_state == false){
    switchDown();
  }

  else if (digitalRead(Sw_1_Pin) == false && lastSW_1_state == true){
    switchOff();
  }


  lastSW_0_state = digitalRead(Sw_0_Pin);
  lastSW_1_state = digitalRead(Sw_1_Pin);
  
  
//        if (!client.connected()) {
//                reconnect();
//        }
//        client.loop();

//        // corrolate Relstate to LedState
//        curRelState=digitalRead(relPin);
//        digitalWrite(ledPin,!curRelState);
//
//        // EXT released (GPIO14)
//        if (digitalRead(extPin)==HIGH) {
//                delay(50);
//                if (digitalRead(extPin)==HIGH && lastRelState!=LOW) {
//                        digitalWrite(relPin,HIGH);
//                        lastRelState=LOW;
//                        client.publish(stateTopic, "on", true);
//                        pub_msg("Ext.Button pressed [ON]");
//                }
//        }
//        // EXT pressed (GPIO14)
//        else if (digitalRead(extPin)==LOW) {
//                delay(50);
//                if (digitalRead(extPin)==LOW && lastRelState!=HIGH) {
//                        digitalWrite(relPin,LOW);
//                        lastRelState=HIGH;
//                        client.publish(stateTopic, "off", true);
//                        pub_msg("Ext.Button pressed [OFF]");
//                }
//        }
//        // Button pressed ( toggle on/off)
//        if (digitalRead(buttonPin)==0) {
//                toggleState = !toggleState;
//                digitalWrite(relPin, toggleState);
//                sprintf(msg, "Button toggled [%s]",toggleState ? "ON" : "OFF");
//                pub_msg(msg);
//                sprintf(msg, "%s",toggleState ? "on" : "off");
//                client.publish(stateTopic, msg, true);
//                delay(300);
//        }
//
//        delay(50);
}
