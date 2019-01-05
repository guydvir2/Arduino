#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>


//MQTT topics - must change for every device
const char *deviceTopic = "HomePi/Dvir/Windows/slave";
const char *stateTopic="HomePi/Dvir/Windows/slave/State";
const char* availTopic = "HomePi/Dvir/Windows/slave/Avail";
const char* deviceName = deviceTopic;
//###########################


// GPIO Pins for ESP8266 - change if needed
const int relUpIndPin = 14;
const int relDownIndPin = 12;
const int CMD2relUpPin = 4;
const int CMD2relDownPin = 5;
//######################


// GPIO status flags
bool relUpIndPin_curState;
bool relDownIndPin_curState;
bool relUpIndPin_lastState;
bool relDownIndPin_lastState;
// ##########################


//wifi creadentials - change if needed
const char* ssid = "HomeNetwork_2.4G";
const char* password = "guyd5161";
//###################################


//MQTT broker parameters
const char* mqtt_server = "192.168.2.200";
const char* user = "guy";
const char* passw = "kupelu9e";
// ######################################


// CONST topics
const char* msgTopic = "HomePi/Dvir/Messages";
const char* groupTopic = "HomePi/Dvir/All";
const char* topicArry[]={deviceTopic,groupTopic};
const char* switchStates[]={"up","down","off"};
// ##############################################


// time interval parameters
const int clockUpdateInt=1; // hrs to update clock
int timeIntResetCounter = 1000; // time between consq presses to init RESET cmd
long MQTTtimeOut = (1000*60)*5; //5 mins stop try to MQTT
long WIFItimeOut = (1000*60)*2; //2 mins try to reconnect WiFi
int debouncetInt = 50;
// ############################

// Board RESET parameters
int manResetCounter = 0;
int pressAmount2Reset = 5;
long lastResetPress = 0;
long resetTimer = 0;
// ####################

// MQTT connection flags
int mqttFailCounter = 0;
long firstNotConnected = 0;
int connectionFlag = 0;
int MQTTretries = 3;
// ######################

// assorted
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
const char *ver="1.3";
// ###################

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

        Serial.begin(9600);

        pinMode(relUpIndPin, INPUT_PULLUP);
        pinMode(relDownIndPin, INPUT_PULLUP);
        
        // Output Pins to switch Arduino Remote input and trigger relay accordingly
        pinMode(CMD2relUpPin, OUTPUT);
        pinMode(CMD2relDownPin, OUTPUT);

        digitalWrite(CMD2relUpPin,HIGH);
        digitalWrite(CMD2relDownPin,HIGH);

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

        // PowerOn Bit
        PBit();
}

void startWifi() {
        long startWifiConnection=0;

        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);

        startWifiConnection = millis();
        WiFi.begin(ssid, password);
        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED && millis()-startWifiConnection <WIFItimeOut) {
                delay(500);
                Serial.print(".");
        }

        WiFi.setAutoReconnect(true);
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
}

int connectMQTT() {
        // verify wifi connected
        if (WiFi.status() == WL_CONNECTED) {
                Serial.println("have wifi, entering MQTT connection");
                while (!client.connected() && mqttFailCounter <= MQTTretries) {
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
                                mqttFailCounter = 0;
                                return 1;
                        }
                        else {
                                Serial.print("failed, rc=");
                                Serial.print(client.state());
                                Serial.println(" try again in 5 seconds");
                                delay(5000);
                                Serial.print("number of fails to reconnect MQTT");
                                Serial.println(mqttFailCounter);
                                mqttFailCounter++;
                        }
                }
                Serial.println("Exit without connecting MQTT");
                mqttFailCounter = 0;
                return 0;
        }
        else {
                Serial.println("Not connected to Wifi, abort try to connect MQTT broker");
                return 0;
        }
}

void callback(char* topic, byte* payload, unsigned int length) {
        char incoming_msg[50];
        char state[5];
        char state2[5];
        char msg2[100];

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
                // relays state
                if (relUpIndPin_curState==LOW && relDownIndPin_curState==LOW) {
                        sprintf(state,"invalid Relay State");
                }
                else if (relUpIndPin_curState==HIGH && relDownIndPin_curState==LOW) {
                        sprintf(state,"DOWN");
                }
                else if (relUpIndPin_curState==LOW && relDownIndPin_curState==HIGH) {
                        sprintf(state,"UP");
                }
                else {
                        sprintf(state,"OFF");
                }

                // switch state
                if (relUpIndPin_lastState==HIGH && relDownIndPin_lastState==HIGH) {
                        sprintf(state2, "OFF");
                }
                else if (relUpIndPin_lastState==LOW && relDownIndPin_lastState==HIGH) {
                        sprintf(state2, "UP");
                }
                else if (relUpIndPin_lastState==HIGH && relDownIndPin_lastState==LOW) {
                        sprintf(state2, "DOWN");
                }

                sprintf(msg,"Status #1: bootTime:[%s]",bootTime);
                pub_msg(msg);
                sprintf(msg2, "Status #2 Relay:[%s],Switch:[%s],Ver:[%s]", state,state2,ver);
                pub_msg(msg2);
        }
//      switch commands via MQTT
        else if (strcmp(incoming_msg,"up")==0 || strcmp(incoming_msg,"down")==0 || strcmp(incoming_msg,"off")==0) {
                switchIt("MQTT",incoming_msg);
        }
        else if (strcmp(incoming_msg,"reset")==0 ) {
                sendReset();
        }
}

void pub_msg(char *inmsg){
        char tmpmsg[150];

        get_timeStamp();
        sprintf(tmpmsg,"[%s] [%s] %s",timeStamp,deviceTopic, inmsg);
        client.publish(msgTopic, tmpmsg);
}

void get_timeStamp(){
        time_t t=now();
        sprintf(timeStamp,"%02d-%02d-%02d %02d:%02d:%02d",year(t),month(t), day(t), hour(t), minute(t), second(t));
}

void switchIt(char *type, char *dir){
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir,"up")==0) {
                states[0]=LOW;
                states[1]=HIGH;
        }
        else if (strcmp(dir,"down")==0) {
                states[0]=HIGH;
                states[1]=LOW;
        }
        else if (strcmp(dir,"off")==0) {
                states[0]=HIGH;
                states[1]=HIGH;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (relUpIndPin_curState != states[0] && relDownIndPin_curState != states[1]) {
                delay(debouncetInt); // debounce
                // read again same values for debounce
                if (digitalRead(relUpIndPin) != states[0] && digitalRead(relDownIndPin) != states[1]) {
                        // switch off both relays
                        digitalWrite(CMD2relUpPin, HIGH);
                        digitalWrite(CMD2relDownPin, HIGH);
                        delay(debouncetInt);
                        // switch on needed relays
                        digitalWrite(CMD2relUpPin, states[0]);
                        digitalWrite(CMD2relDownPin, states[1]);
                }
        }

        // Case that only one relay changes from/to off --> on
        else if (relUpIndPin_curState != states[0] || relDownIndPin_curState != states[1]) {
                delay(debouncetInt); //debounce
                // read again for debounce
                if (digitalRead(relUpIndPin) != states[0] || digitalRead(relDownIndPin) != states[1]) {
                        digitalWrite(CMD2relUpPin, states[0]);
                        digitalWrite(CMD2relDownPin, states[1]);
                }
        }

        // updates relay states
        relUpIndPin_curState=states[0];
        relDownIndPin_curState=states[1];

        client.publish(stateTopic, dir, true);
        sprintf(mqttmsg,"[%s] switched [%s]",type, dir);
        pub_msg(mqttmsg);
}

void detectResetPresses(){
        if (millis()-lastResetPress < timeIntResetCounter) {
                Serial.println(millis()-lastResetPress);
                if (manResetCounter >=pressAmount2Reset) {
                        sendReset();
                        manResetCounter=0;
                }
                else {
                        manResetCounter++;
                }
        }
        else {
                manResetCounter = 0;
        }
}

void sendReset() {
        Serial.println("Sending Reset command");
        ESP.reset();
}

void PBit(){
        switchIt("Button","up");
        delay(1000);
        switchIt("Button","down");
        delay(1000);
        switchIt("Button","off");
}

void loop() {
//  MQTT reconnection for first time or after first insuccess to reconnect
        if (!client.connected() && firstNotConnected == 0) {
                connectionFlag=connectMQTT();
//  still not connected
                if (connectionFlag == 0 ) {
                        firstNotConnected=millis();
                }
                else {
                        client.loop();
                }
        }
// retry after fail - resume only after timeout
        else if (!client.connected() && firstNotConnected !=0 && millis() - firstNotConnected > MQTTtimeOut) {
//    after cooling out period - try again
                connectionFlag=connectMQTT();
                firstNotConnected = 0;
                Serial.println("trying again to reconnect");
        }
        else {
                client.loop();
        }
// #############################

// read state of local indication GPIO (reflects Relays state on remote Arduino board)
        relUpIndPin_curState = digitalRead(relUpIndPin);
        relDownIndPin_curState = digitalRead(relDownIndPin);

//  verfiy not in Hazard State
        if (relUpIndPin_curState == LOW && relDownIndPin_curState == LOW ) {
                switchIt("Button","off");
                Serial.println("Hazard state - both switches were ON");
        }
        // ###########################################

// check for changes done remotely on Arduino board and need to notify MQTT using ESP8266:
// up
        if (relUpIndPin_curState != relUpIndPin_lastState) {
                delay(debouncetInt);
                if (digitalRead(relUpIndPin)!= relUpIndPin_lastState) {
                        pub_msg("[Local] Switch [UP]");
                }
        }
// down
        if (relDownIndPin_curState != relDownIndPin_lastState) {
                delay(debouncetInt);
                if (digitalRead(relDownIndPin)!= relDownIndPin_lastState) {
                        pub_msg("[Local] Switch [DOWN]");
                }
        }
// ###############################################


        relUpIndPin_lastState = digitalRead(relUpIndPin);
        relDownIndPin_lastState = digitalRead(relDownIndPin);
        delay(50);
}
