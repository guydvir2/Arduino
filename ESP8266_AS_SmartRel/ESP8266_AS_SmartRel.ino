//#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

//wifi creadentials - change if needed
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
//###################################

// GPIO Pins - change if needed
const int Sw_0_Pin = 2;
const int Sw_1_Pin = 3;
const int Rel_0_Pin = 4;
const int Rel_1_Pin = 5;
//######################

//MQTT topics - change for every device 
const char* deviceName = "test1_esp";
const char* deviceTopic = "HomePi/Dvir/ESP/test";
const char* stateTopic = "HomePi/Dvir/ESP/test/State";
const char* availTopic = "HomePi/Dvir/ESP/Stest/Avail";
//#####################################################

//MQTT broker parameters
const char* mqtt_server = "192.168.2.200";
const char* user = "guy";
const char* passw = "kupelu9e";

// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
const char* groupTopic = "HomePi/Dvir/All";
const char* topicArry[]={deviceTopic,groupTopic};

char msg[150];
char timeStamp[50];
char bootTime[50];

// GPIO status flags
bool Rel_0_state = false;
bool Rel_1_state = false;
bool lastSW_0_state = false;
bool lastSW_1_state = false;

bool firstRun = true;
char *ver="0.1";
const int clockUpdateInt=1;

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

//in case of change -- > there is a retain msg to change !

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
          if (Rel_0_state==false && Rel_1_state==false){
            sprintf(state,"OFF");
          }
          else if (Rel_0_state==true && Rel_1_state==false){
            sprintf(state,"UP");
          }
          else if (Rel_0_state==false && Rel_1_state==true){
            sprintf(state,"DOWN");
          }
          else {
            sprintf(state,"invalid");
          }
  
        sprintf(msg,"Status CMD: Relay State: [%d], bootTime: [%s], [v%s]",state,bootTime,ver);
        pub_msg(msg);
        }
//      up - via MQTT
        else if (strcmp(incoming_msg,"up")==0) {
                switchUp();
                sprintf(msg,"Topic: [%s] Remote CMD: [UP]",topic);
                pub_msg(msg);
        }
//      off - via MQTT
        else if (strcmp(incoming_msg,"off")==0) {
                switchOff();
                sprintf(msg,"Topic: [%s] Remote CMD: [OFF]",topic);
                pub_msg(msg);
        }
//      down - via MQTT
        else if (strcmp(incoming_msg,"down")==0) {
                switchDown();
                sprintf(msg,"Topic: [%s] Remote CMD: [DOWN]",topic);
                pub_msg(msg);
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
  digitalWrite(Rel_1_Pin, LOW);
  Rel_1_state=false;
  delay(50);
  digitalWrite(Rel_0_Pin, HIGH);
  Rel_0_state=true;

  client.publish(stateTopic, "up", true);
  pub_msg("pressed [UP]");
}

void switchDown(){
  digitalWrite(Rel_0_Pin, false);
  Rel_0_state=false;
  delay(50);
  digitalWrite(Rel_1_Pin, true);
  Rel_1_state=true;
  
  client.publish(stateTopic, "down", true);
  pub_msg("pressed [DOWN]");
}

void switchOff(){
  upOff();
  delay(50);
  downOff();
  
  client.publish(stateTopic, "off", true);
  pub_msg("pressed [OFF]");
}

void upOff(){
  digitalWrite(Rel_0_Pin, false);
  Rel_0_state=false;
}

void downOff(){
  digitalWrite(Rel_1_Pin, false);
  Rel_1_state=false;
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

  
//  physical switch change detected

//  switch UP
  if (digitalRead(Sw_0_Pin) != lastSW_0_state){
    delay(50); //debounce
    if (digitalRead(Sw_0_Pin) != lastSW_0_state){
      if (digitalRead(Sw_0_Pin) == HIGH && Rel_0_state!=true){
        switchUp();
      }
      else if (digitalRead(Sw_0_Pin) == LOW && Rel_0_state!=false){
        upOff();
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
      if (digitalRead(Sw_1_Pin) == HIGH && Rel_1_state!=true){
        switchDown();
      }
      else if (digitalRead(Sw_1_Pin) == LOW && Rel_1_state!=false){
        downOff();
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
