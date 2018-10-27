//#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>


//MQTT topics - must change for every device 
const char *deviceTopic = "HomePi/Dvir/Windows/KidsRoom";
const char *stateTopic="HomePi/Dvir/Windows/KidsRoom/State";
const char* availTopic = "HomePi/Dvir/Windows/KidsRoom/Avail";
const char* deviceName = deviceTopic;
//###########################


// GPIO Pins for ESP8266 - change if needed
const int Sw_0_Pin = 0;
const int Sw_1_Pin = 2;
const int Rel_0_Pin = 4;
const int Rel_1_Pin = 5;
//######################


//wifi creadentials - change if needed
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
//###################################


//MQTT broker parameters
const char* mqtt_server = "192.168.2.200";
const char* user = "guy";
const char* passw = "kupelu9e";


// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
const char* groupTopic = "HomePi/Dvir/All";
const char* topicArry[]={deviceTopic,groupTopic};


// GPIO status flags
bool Rel_0_state;
bool Rel_1_state;
bool lastSW_0_state;
bool lastSW_1_state;

char msg[150];
char timeStamp[50];
char bootTime[50];

bool firstRun = true;
const char *ver="1.0";
const int clockUpdateInt=1; // hrs to update clock

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

        Serial.begin(9600);

        pinMode(Sw_0_Pin, INPUT_PULLUP);
        pinMode(Sw_1_Pin, INPUT_PULLUP);
        pinMode(Rel_0_Pin, OUTPUT);
        pinMode(Rel_1_Pin, OUTPUT);

        digitalWrite(Rel_0_Pin,HIGH);
        digitalWrite(Rel_1_Pin,HIGH);

        startWifi();
//        start MQTT services
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
        
//        start NTP clock updates
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000*3600*clockUpdateInt);
        
//        get boot time stamp
        get_timeStamp();
        strcpy(bootTime,timeStamp);
}

void startWifi() {
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

void connectMQTT() {
        // Loop until we're reconnected
        while (!client.connected()) {
                Serial.print("Attempting MQTT connection...");
                // Attempt to connect
                if (client.connect(deviceName,user, passw, availTopic,0,true,"offline")) {
                        Serial.println("connected");
                        client.publish(availTopic, "online", true);
                        if (firstRun == true) {
                          client.publish(stateTopic, "off", true);
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

void callback(char* topic, byte* payload, unsigned int length) {
        char incoming_msg[50];
        char state[5];
        
//      Display on Serial monitor only
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length]=0;
        Serial.println("");
//      ##############################
        
//      status - via MQTT
        if (strcmp(incoming_msg,"status")==0) {
          if (Rel_0_state==LOW && Rel_1_state==LOW){
            sprintf(state,"invalid MQTT");
          }
          else if (Rel_0_state==HIGH && Rel_1_state==LOW){
            sprintf(state,"DOWN");
          }
          else if (Rel_0_state==LOW && Rel_1_state==HIGH){
            sprintf(state,"UP");
          }
          else {
            sprintf(state,"OFF");
          }
          sprintf(msg,"Status CMD: Relay State: [%s], bootTime: [%s], [v%s]",state,bootTime,ver);
//          pub_msg(msg);
        }
//      up - via MQTT
        else if (strcmp(incoming_msg,"up")==0 || strcmp(incoming_msg,"down")==0 || strcmp(incoming_msg,"off")==0) {
          switchIt("MQTT",incoming_msg);
        }
//
//        }
////      off - via MQTT
//        else if (strcmp(incoming_msg,"off")==0) {
//          switchIt("MQTT","off");
//        }
////      down - via MQTT
//        else if (strcmp(incoming_msg,"down")==0) {
//          switchIt("MQTT","down");
//        }
//        Serial.println(msg);
}

void pub_msg(char *inmsg){
        char tmpmsg[150];
        char tmpmsg2[150];
        get_timeStamp();
        
        sprintf(tmpmsg,"[%s] [%s] %s",timeStamp,deviceTopic, inmsg);
//        Serial.println(strlen(tmpmsg));
//        if (strlen(tmpmsg)<100){
          client.publish(msgTopic, tmpmsg);
//        }
//        else {
//          tmpmsg[100]=0;
//          client.publish(msgTopic, tmpmsg);
//          Serial.println(tmpmsg);
//        }
}

void get_timeStamp(){
        time_t t=now();
        sprintf(timeStamp,"%02d-%02d-%02d %02d:%02d:%02d:00",year(t),month(t), day(t), hour(t), minute(t), second(t));
}

void switchIt(char *type, char *dir){
  char mqttmsg[50];
  bool states[2];
  if (strcmp(dir,"up")==0){
    states[0]=LOW;
    states[1]=HIGH;
  }
  else if (strcmp(dir,"down")==0){
    states[0]=HIGH;
    states[1]=LOW;
  }
  else if (strcmp(dir,"off")==0){
    states[0]=HIGH;
    states[1]=HIGH;
  }

  if (Rel_0_state != states[0] && Rel_1_state != states[1]){
    digitalWrite(Rel_0_Pin, HIGH);
    digitalWrite(Rel_1_Pin, HIGH);
    delay(50);
    digitalWrite(Rel_0_Pin, states[0]);
    digitalWrite(Rel_1_Pin, states[1]);
    }
  else if (Rel_0_state != states[0] || Rel_1_state != states[1]){
    digitalWrite(Rel_0_Pin, states[0]);
    digitalWrite(Rel_1_Pin, states[1]);
    }
    
  Rel_1_state=states[0];
  Rel_0_state=states[1];

  client.publish(stateTopic, dir, true);
  sprintf(mqttmsg,"[%s] switched [%s]" ,type, dir);
  pub_msg(mqttmsg);
}

void loop() {
  
//  MQTT service
  if (!client.connected()) {
    connectMQTT();
    }
    client.loop();
//    ##
    
  Rel_0_state = digitalRead(Rel_0_Pin);
  Rel_1_state = digitalRead(Rel_1_Pin);

//  verfiy not in Hazard State
  if (Rel_0_state == LOW && Rel_1_state == LOW ){
    switchIt("Button","off");
    Serial.println("Hazard state - both switches were ON");
  }
//  ##

//  physical switch change detected

//  switch UP
  if (digitalRead(Sw_0_Pin) != lastSW_0_state){
    delay(50); //debounce
    if (digitalRead(Sw_0_Pin) != lastSW_0_state){
      if (digitalRead(Sw_0_Pin) == LOW && Rel_0_state!=LOW){
        switchIt("Button","up");
      }
      else if (digitalRead(Sw_0_Pin) == HIGH && Rel_0_state!=HIGH){
        switchIt("Button","off");
      }
      else {
        Serial.println("Wrong command");
      }
    }
  }

//  switch down
  if (digitalRead(Sw_1_Pin) != lastSW_1_state){
    delay(50);
    if (digitalRead(Sw_1_Pin) != lastSW_1_state){
      if (digitalRead(Sw_1_Pin) == LOW && Rel_1_state!=LOW){
        switchIt("Button","down");        
      }
      else if (digitalRead(Sw_1_Pin) == HIGH && Rel_1_state!=HIGH){
        switchIt("Button","off");        
      }
      else {
        Serial.println("Wrong command");
      }
    }
  }


  lastSW_0_state = digitalRead(Sw_0_Pin);
  lastSW_1_state = digitalRead(Sw_1_Pin);

  delay(50);
}
