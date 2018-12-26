//###################################################
#define deviceTopic "HomePi/Dvir/WateringSystem"
const char *ver = "WEMOS_WDT_OTA_2.71";
//###################################################

// Service flags
bool useNetwork = true;
bool useWDT = true;
bool useSerial = false;
bool useOTA = true;
bool runPbit = false;
int networkID = 1;  // 0: HomeNetwork,  1:Xiaomi_D6C8

#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <PubSubClient.h> //MQTT
#include <Ticker.h> //WDT

// OTA libraries
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// #######################


//wifi creadentials
const char* ssid;
const char* ssid_0 = "HomeNetwork_2.4G";
const char* ssid_1 = "Xiaomi_D6C8";
const char* password = "guyd5161";
//###################################


//MQTT broker parameters
const char* mqtt_server;
const char* mqtt_server_0 = "192.168.2.200";
const char* mqtt_server_1 = "192.168.3.200";
const char* user = "guy";
const char* passw = "kupelu9e";
// ######################################


// MQTT topics
const char* msgTopic = "HomePi/Dvir/Messages";
const char* groupTopic = "HomePi/Dvir/All";
const char* deviceName = deviceTopic;
const char* topicArry[] = {deviceTopic, groupTopic};
char stateTopic[50];
char availTopic[50];
// ##############################################


// MQTT connection flags
int mqttFailCounter = 0; // count tries to reconnect
int MQTTretries = 2; // allowed tries to reconnect
bool mqttConnected = 0;
// ######################


// time interval parameters
const int clockUpdateInt = 1; // hrs to update NTP
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
const long WIFItimeOut = (1000 * 60) * 0.5; // 1/2 mins try to connect WiFi
const long OTA_upload_interval = (1000*60) * 2; // 2 minute to try OTA
const int time2Reset_noNetwork = (1000*60)*5; // minutues pass without any network
const int time2_tryReconnect = (1000*60)*1; // time between reconnection retries

const int deBounceInt = 50; //
volatile int wdtResetCounter = 0;
const int wdtMaxRetries = 20; //seconds to bITE
long noNetwork_Counter=0; // clock
long OTAcounter =0; // clock
// ############################


// manual RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 3; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
unsigned long lastPressTime_1 = 0;
unsigned long lastPressTime_2 = 0;
int delayBetweenPress = 1000;
// ####################


// hold informamtion
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
// ###################


WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker wdt;

// Code Specific #####################################

// GPIO Pins for ESP8266
const int input_1Pin = 14;
const int input_2Pin = 12;
const int output_1Pin = 4;
const int output_2Pin = 5;
//##########################

// GPIO status flags
bool output1_currentState;
bool output2_currentState;
// ###########################


void setup() {
        startGPIOs();
        if (useSerial) {
                Serial.begin(9600);
                delay(10);
                Serial.println("SystemBoot");
        }
        if (useNetwork) {
                startNetwork();
        }
        if(useOTA) {
                startOTA();
        }
        if (useWDT) {
                wdt.attach(1, feedTheDog); // Start WatchDog
        }
        if (runPbit) {
                PBit(); // PowerOn Bit
        }
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


// Common ##############
// ~~~~~~~ Network connectivity ~~~~~
void selectNetwork() {
        if (networkID == 1 ) {
                ssid = ssid_1;
                mqtt_server = mqtt_server_1;
        }
        else {
                ssid = ssid_0;
                mqtt_server = mqtt_server_0;
        }
}
void startNetwork() {
        long startWifiConnection = millis();

        selectNetwork();
        if (useSerial) {
                Serial.println();
                Serial.print("Connecting to ");
                Serial.println(ssid);
        }

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        WiFi.setAutoReconnect(true);

        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED && millis() - startWifiConnection < WIFItimeOut) {
                delay(500);
                if (useSerial) {
                        Serial.print(".");
                }
        }

        // case of no success - restart due to no wifi
        if (WiFi.status() != WL_CONNECTED) {
                if (useSerial) {
                        Serial.println("no wifi detected");
                }
        }

        // if wifi is OK
        else {
                if (useSerial) {
                        Serial.println("");
                        Serial.println("WiFi connected");
                        Serial.print("IP address: ");
                        Serial.println(WiFi.localIP());
                }

                startMQTT();
                startNTP();
                get_timeStamp();
                strcpy(bootTime, timeStamp);
                subscribeMQTT();
        }
}
int networkStatus(){
        if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
                mqttClient.loop();
                mqttConnected = 1;
                // noNetwork_Counter = 0;
                return 1;
        }
        else {
                if (noNetwork_Counter == 0) {
                        noNetwork_Counter = millis();
                }
                mqttConnected = 0;
                return 0;
        }
}
void network_check(){
        if ( networkStatus() == 0) {
                if (millis()-noNetwork_Counter >= time2Reset_noNetwork) {
                        sendReset("null");
                }
                if (millis()-noNetwork_Counter >= time2_tryReconnect) {
                        startNetwork();
                        noNetwork_Counter = 0;
                }
        }
}

// ~~~~ Services ~~~~~~
void startNTP() {
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000 * 3600 * clockUpdateInt);
}
void startOTA() {
        char OTAname[100];
        int m = 0;
        // create OTAname from deviceTopic
        for (int i = ((String)deviceTopic).lastIndexOf("/") + 1; i < strlen(deviceTopic); i++) {
                OTAname[m] = deviceTopic[i];
                OTAname[m + 1] = '\0';
                m++;
        }

        OTAcounter = millis();

        // Port defaults to 8266
        ArduinoOTA.setPort(8266);

        // Hostname defaults to esp8266-[ChipID]
        ArduinoOTA.setHostname(OTAname);

        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        // Password can be set with it's md5 value as well
        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
        // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

        ArduinoOTA.onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH) {
                        type = "sketch";
                } else { // U_SPIFFS
                        type = "filesystem";
                }

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                if (useSerial) {
                        Serial.println("Start updating " + type);
                }
                // Serial.end();
        });
        if (useSerial) { // for debug
                ArduinoOTA.onEnd([]() {
                        Serial.println("\nEnd");
                });
                ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
                });
                ArduinoOTA.onError([](ota_error_t error) {
                        Serial.printf("Error[%u]: ", error);
                        if (error == OTA_AUTH_ERROR) {
                                Serial.println("Auth Failed");
                        } else if (error == OTA_BEGIN_ERROR) {
                                Serial.println("Begin Failed");
                        } else if (error == OTA_CONNECT_ERROR) {
                                Serial.println("Connect Failed");
                        } else if (error == OTA_RECEIVE_ERROR) {
                                Serial.println("Receive Failed");
                        } else if (error == OTA_END_ERROR) {
                                Serial.println("End Failed");
                        }
                });
                // ArduinoOTA.begin();
                Serial.println("Ready");
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());
        }

        ArduinoOTA.begin();
}

// ~~~~~~~ MQTT functions ~~~~~~~
void startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        mqttClient.setServer(mqtt_server, 1883);
        mqttClient.setCallback(callback);
}
int subscribeMQTT() {
        long startClock = millis();

        // verify wifi connected
        if (WiFi.status() == WL_CONNECTED) {
                if (useSerial) {
                        Serial.println("have wifi, entering MQTT connection");
                }
                while (!mqttClient.connected() && mqttFailCounter <= MQTTretries) {
                        if (useSerial) {
                                Serial.print("Attempting MQTT connection...");
                        }

                        // Attempt to connect
                        if (mqttClient.connect(deviceName, user, passw, availTopic, 0, true, "offline")) {
                                if (useSerial) {
                                        Serial.println("connected");
                                }
                                mqttConnected = 1;
                                mqttClient.publish(availTopic, "online", true);
                                if (firstRun == true) {
                                        mqttClient.publish(stateTopic, "off", true);
                                        firstRun = false;
                                }
                                pub_msg("Connected to MQTT server");
                                for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++) {
                                        mqttClient.subscribe(topicArry[i]);
                                        sprintf(msg, "Subscribed to %s", topicArry[i]);
                                }
                                mqttFailCounter = 0;
                                return 1;
                        }

                        // fail to connect, but have few retries
                        else {
                                if (useSerial) {
                                        Serial.print("failed, rc=");
                                        Serial.print(mqttClient.state());
                                        Serial.print("number of fails to reconnect MQTT :");
                                        Serial.println(mqttFailCounter);
                                }
                                mqttFailCounter++;
                        }
                }

                // Failed to connect MQTT adter retries
                if (useSerial) {
                        Serial.println("Exit without connecting MQTT");
                }
                mqttFailCounter = 0;
                return 0;
        }
        else {
                if (useSerial) {
                        Serial.println("Not connected to Wifi, abort try to connect MQTT broker");
                }
                return 0;
        }
}
void createTopics(const char *devTopic, char *state, char *avail) {
        sprintf(state, "%s/State", devTopic);
        sprintf(avail, "%s/Avail", devTopic);
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
void pub_msg(char *inmsg) {
        char tmpmsg[150];

        if (useNetwork == true && mqttConnected == true) {
                get_timeStamp();
                sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic );
                msgSplitter(inmsg, 95, tmpmsg, "#" );
        }
}
void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg) {
        char tmp[120];

        if (strlen(prefix) + strlen(msg_in) > max_msgSize) {
                int max_chunk = max_msgSize - strlen(prefix) - strlen(split_msg);
                int num = ceil((float)strlen(msg_in) / max_chunk);
                int pre_len;

                for (int k = 0; k < num; k++) {
                        sprintf(tmp, "%s %s%d: ", prefix, split_msg, k);
                        pre_len = strlen(tmp);
                        for (int i = 0; i < max_chunk; i++) {
                                tmp[i + pre_len] = (char)msg_in[i + k * max_chunk];
                                tmp[i + 1 + pre_len] = '\0';
                        }
                        if (useNetwork && mqttConnected == true) {
                                mqttClient.publish(msgTopic, tmp);
                        }
                }
        }
        else {  if (useNetwork && mqttConnected == true) {
                        sprintf(tmp, "%s %s", prefix, msg_in);
                        mqttClient.publish(msgTopic, tmp);
                }}
}
void get_timeStamp() {
        time_t t = now();
        sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}

// ~~~~~~ Reset and maintability ~~~~~~
void sendReset(char *header) {
        char temp[150];

        if (useSerial) {
                Serial.println("Sending Reset command");
        }
        if (strcmp(header, "null")!=0) {
                sprintf(temp, "[%s] - Reset sent", header);
                pub_msg(temp);
        }
        ESP.reset();
}
void feedTheDog() {
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset("WatchDog woof");
        }
}
void acceptOTA() {
        if (millis() - OTAcounter <= OTA_upload_interval) {
                ArduinoOTA.handle();
        }
}
// END Common ############


// Code specific #######################
// ~~~~ maintability ~~~~~~
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
void allOff() {
        digitalWrite(output_1Pin,HIGH); //relay off
        digitalWrite(output_2Pin,HIGH);
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
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

        mqttClient.publish(stateTopic, dir, true);
        sprintf(mqttmsg,"[%s] switched [%s]",type, dir);
        pub_msg(mqttmsg);
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
void detectResetPresses() {
        if (millis() - lastResetPress < timeInterval_resetPress) {
                if (useSerial) {
                        Serial.print("Time between Press: ");
                        Serial.println(millis() - lastResetPress);
                }
                if (manResetCounter >= pressAmount2Reset) {
                        sendReset("Manual operation");
                        if (useSerial) {
                                Serial.println("Manual Reset initiated");
                        }
                        manResetCounter = 0;
                }
                else {
                        manResetCounter++;
                }
        }
        else {
                manResetCounter = 0;
        }
}
void readGpioStates(){
        output1_currentState = digitalRead(output_1Pin);
        output2_currentState = digitalRead(output_2Pin);
}

void loop() {
        readGpioStates();

        // Service updates
        if (useNetwork) {
                network_check();
        }
        if (useWDT) {
                wdtResetCounter = 0;
        }
        if (useOTA) {
                acceptOTA();
        }

        checkSwitch_1();
        checkSwitch_2();

        delay(50);
}
