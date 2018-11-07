//change deviceTopic !
//###################################################
#define deviceTopic "HomePi/Dvir/WateringSystem"
//###################################################

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <Ticker.h>


// GPIO Pins for ESP8266
const int input_1Pin = 14;
const int input_2Pin = 12;
const int output_1Pin = 4;
const int output_2Pin = 5;
//##########################


//wifi creadentials
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
const char* deviceName = deviceTopic;
const char* topicArry[]={deviceTopic,groupTopic};
const char* switchStates[]={"on","off"};
char stateTopic[50];
char availTopic[50];
// ##############################################


// GPIO status flags
bool output1_currentState;
bool output2_currentState;
// ###########################

// time interval parameters
const int clockUpdateInt=1; // hrs to update clock
int timeInt2Reset = 2000; // time between consq presses to init RESET cmd
long MQTTtimeOut = (1000*60)*5; //5 mins stop try to MQTT
long WIFItimeOut = (1000*60)*2; //2 mins try to connect WiFi
int deBounceInt = 100; // ---> extend from 50 to 100 ms
volatile int wdtResetCounter = 0;
int wdtMaxRetries =3;
unsigned long lastPressTime_1 = 0;
unsigned long lastPressTime_2 = 0;
int delayBetweenPress = 1000;
// ############################

// RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 5; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
long resetTimer = 0;
// ####################

// MQTT connection flags
int mqttFailCounter = 0; // count tries to reconnect
long firstNotConnected = 0; // time stamp of first try
int connectionFlag = 0;
int MQTTretries = 3; // allowed tries to reconnect
// ######################

// assorted
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
const char *ver="1.8";
// ###################


WiFiClient espClient;
PubSubClient client(espClient);
Ticker wdt;


void setup() {
        Serial.begin(9600);

        startGPIOs();
        startNetwork();
        PBit(); // PowerOn Bit
        wdt.attach(1,takeTheDog); // Start WatchDog

}

void startGPIOs(){
        pinMode(input_1Pin, INPUT_PULLUP);
        pinMode(input_2Pin, INPUT_PULLUP);
        pinMode(output_1Pin, OUTPUT);
        pinMode(output_2Pin, OUTPUT);
        /*
        input == LOW is switch on
        rel == HIGH is switched off
         */

        allOff();
}

void startNetwork() {
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

        startMQTT();
        startNTP();

        get_timeStamp();
        strcpy(bootTime,timeStamp);
}

void startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
}

void startNTP() {
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000*3600*clockUpdateInt);
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

void createTopics(const char *devTopic, char *state, char *avail) {
        sprintf(state,"%s/State",devTopic);
        sprintf(avail,"%s/Avail",devTopic);
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
                sprintf(msg,"Status: bootTime:[%s], Relay#1:[%d], Relay#2[%d], Ver:[%s]",bootTime, digitalRead(output_1Pin),digitalRead(output_1Pin),ver);
                pub_msg(msg);
        }
//      switch commands via MQTT
        else if (strcmp(incoming_msg,"1,on")==0 ) {
                switchIt("MQTT","1,on");
        }
        else if (strcmp(incoming_msg,"1,off")==0) {
                switchIt("MQTT","1,off");
        }
        else if (strcmp(incoming_msg,"2,on")==0 ) {
                switchIt("MQTT","2,on");
        }
        else if (strcmp(incoming_msg,"2,off")==0) {
                switchIt("MQTT","2,off");
        }
}

void pub_msg(char *inmsg){
        char tmpmsg[150];

        get_timeStamp();
        sprintf(tmpmsg,"[%s] [%s]",timeStamp,deviceTopic );
        msgSplitter(inmsg, 95, tmpmsg, "split #" );
}

void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg){
        char tmp[120];

        if (strlen(prefix) + strlen(msg_in) > max_msgSize) {
                int max_chunk = max_msgSize - strlen(prefix) - strlen(split_msg);
                int num=ceil((float)strlen(msg_in)/max_chunk);
                int pre_len;

                for (int k=0; k<num; k++) {
                        sprintf(tmp,"%s %s%d: ",prefix,split_msg,k);
                        pre_len = strlen(tmp);
                        for (int i=0; i<max_chunk; i++) {
                                tmp[i+pre_len]=(char)msg_in[i+k*max_chunk];
                                tmp[i+1+pre_len]='\0';
                        }
                        client.publish(msgTopic, tmp);

                }
        }
        else {
                sprintf(tmp,"%s %s",prefix,msg_in);
                client.publish(msgTopic, tmp);
        }
}

void get_timeStamp(){
        time_t t=now();
        sprintf(timeStamp,"%02d-%02d-%02d %02d:%02d:%02d",year(t),month(t), day(t), hour(t), minute(t), second(t));
}

void switchIt(char *type, char *dir){
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir,"1,on")==0) {
                digitalWrite(output_1Pin,LOW);
        }
        else if (strcmp(dir,"1,off")==0) {
                digitalWrite(output_1Pin,HIGH);
        }
        else if (strcmp(dir,"2,on")==0) {
                digitalWrite(output_2Pin,LOW);
        }
        else if (strcmp(dir,"2,off")==0) {
                digitalWrite(output_2Pin,HIGH);
        }

        client.publish(stateTopic, dir, true);
        sprintf(mqttmsg,"[%s] switched [%s]",type, dir);
        pub_msg(mqttmsg);
}

void detectResetPresses(){
        if (millis()-lastResetPress < timeInt2Reset) {
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
        ESP.restart();
}

void PBit(){
        allOff();
        digitalWrite(output_1Pin, LOW);
        delay(10);
        digitalWrite(output_1Pin, HIGH);
        delay(10);
        digitalWrite(output_2Pin, LOW);
        delay(10);
        allOff();

}

void verifyMQTTConnection(){
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
}

void allOff() {
        digitalWrite(output_1Pin,HIGH); //relay off
        digitalWrite(output_2Pin,HIGH);
}

void checkSwitch_1() {
        if (digitalRead(input_1Pin) == LOW) {
                delay(deBounceInt);
                if (digitalRead(input_1Pin) == LOW && millis() - lastPressTime_1 >delayBetweenPress) {
                        if (output1_currentState == HIGH ) {
                                switchIt("Button", "1,on");
                        }
                        else {
                                switchIt("Button", "1,off");
                        }
                        lastPressTime_1 = millis();
                        detectResetPresses();
                        lastResetPress = millis();
                }

        }
}

void checkSwitch_2(){
        if (digitalRead(input_2Pin) == LOW) {
                delay(deBounceInt);
                if (digitalRead(input_2Pin) == LOW && millis() - lastPressTime_2 >delayBetweenPress) {
                        if (output2_currentState == HIGH) {
                                switchIt("Button", "2,on");
                        }
                        else {
                                switchIt("Button", "2,off");
                        }
                        lastPressTime_2 = millis();
                }

        }
}

void takeTheDog(){
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset();
        }
}

void readGpioStates(){
        output1_currentState = digitalRead(output_1Pin);
        output2_currentState = digitalRead(output_2Pin);
}

void loop() {
        wdtResetCounter = 0; // reset WDT

        verifyMQTTConnection();
        readGpioStates();
        checkSwitch_1();
        checkSwitch_2();

        delay(50);
}
