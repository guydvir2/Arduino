/*
   This code is a part of 2 codes system: 1) runs on arduino and 2) ESP8266 (this code)
   Both devices are ment to control an Up-Off-Down electric window cover.
   The use of 2 components, an Arduino and an ESP8266 is for reliablity pusposes.
   Arduino is incharge of commanding the dual relay ( Up/Off/Down) using a physial switch.
   ESP8266 is ment to get remote commands via a MQTT protocol and via gpio flaging the arduino to
   switch the window accordingly.

   The main reason for using the Arduino - is that in case of some wifi/MQTT/code crash - to be able
   to person to shut the cover ( as said- reliablity).

   Written : Guy Dvir
   Date: 10/2018
 */


//change deviceTopic !
//###################################################
#define deviceTopic "Dvir/Windows/GardenExit"

// Service flags
bool useNetwork = true;
bool useWDT = true;
bool useSerial = false;
bool runPbit = true;


const char *ver = "ESP_AdruinoMaster_1.1";
//###################################################

// GPIO Pins for ESP8266
#define inputUpPin 14
#define inputDownPin 12
#define outputUpPin 4
#define outputDownPin 5
//######################

// Hw states
#define RelayOn LOW
#define SwitchOn HIGH
//####################

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <Ticker.h>

// OTA libraries
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// #######################


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
const char* msgTopic = "Dvir/Messages";
const char* groupTopic = "Dvir/All";
const char* deviceName = deviceTopic;
const char* topicArry[] = {deviceTopic, groupTopic};
char stateTopic[50];
char availTopic[50];
// ##############################################


// GPIO status flags
bool outputUp_currentState;
bool outputDown_currentState;
bool inputUp_lastState;
bool inputDown_lastState;
bool inputUp_currentState;
bool inputDown_currentState;
// ###########################

// time interval parameters
const int clockUpdateInt = 1; // hrs to update clock
int timeInt2Reset = 1500; // time between consq presses to init RESET cmd
int MQTTtimeOut = (1000 * 60) * 5; //5 mins stop try to MQTT
int WIFItimeOut = (1000 * 60) * 2; //2 mins try to connect WiFi
int MQTTtimeOut_repost = 500; // 500 mil.seconds delay between switch post and MQTT command
unsigned long repostCounter = 0;
int deBounceInt = 100;

volatile int wdtResetCounter = 0;
int wdtMaxRetries = 5; // seconds before bITE
// ############################

// RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 5; // time to press button to init Reset
unsigned long lastResetPress = 0; // time stamp of last press
unsigned long resetTimer = 0;
// ####################

// MQTT connection flags
int mqttFailCounter = 0; // count tries to reconnect
unsigned long firstNotConnected = 0; // time stamp of first try
int connectionFlag = 0;
int MQTTretries = 3; // retries to reconnect
// ######################

// assorted
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
// const char *ver = "Ard+laveESP_1.1";
// ###################


WiFiClient espClient;
PubSubClient client(espClient);
Ticker wdt;


void setup() {
        startGPIOs();
        if (useSerial) {
                Serial.begin(9600);
        }
        if (useNetwork) {
                startNetwork();
        }
        if (runPbit) {
                PBit(); // PowerOn Bit
        }
        if (useWDT) {
                wdt.attach(1, takeTheDog);
        }
}

void startGPIOs() {
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        allOff();
}

void startNetwork() {
        long startWifiConnection = 0;
        if (useSerial) {
                Serial.println();
                Serial.print("Connecting to ");
                Serial.println(ssid);
        }

        startWifiConnection = millis();
        WiFi.begin(ssid, password);
        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED && millis() - startWifiConnection < WIFItimeOut) {
                delay(500);
                if (useSerial) {
                        Serial.print(".");
                }
        }

        WiFi.setAutoReconnect(true);
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
}

void startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
}

void startNTP() {
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000 * 3600 * clockUpdateInt);
}

int connectMQTT() {
        // verify wifi connected
        if (WiFi.status() == WL_CONNECTED) {
                if (useSerial) {
                        Serial.println("have wifi, entering MQTT connection");
                }
                while (!client.connected() && mqttFailCounter <= MQTTretries) {
                        if (useSerial) {
                                Serial.print("Attempting MQTT connection...");
                        }
                        // Attempt to connect
                        if (client.connect(deviceName, user, passw, availTopic, 0, true, "offline")) {
                                if (useSerial) {
                                        Serial.println("connected");
                                }
                                client.publish(availTopic, "online", true);
                                if (firstRun == true) {
                                        client.publish(stateTopic, "off", true);
                                        firstRun = false;
                                }
                                pub_msg("Connected to MQTT server");
                                for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++) {
                                        client.subscribe(topicArry[i]);
                                        sprintf(msg, "Subscribed to %s", topicArry[i]);
                                }
                                mqttFailCounter = 0;
                                return 1;
                        }
                        else {
                                if (useSerial) {
                                        Serial.print("failed, rc=");
                                        Serial.print(client.state());
                                        Serial.println(" try again in 5 seconds");
                                }
                                delay(5000);
                                if (useSerial) {
                                        Serial.print("number of fails to reconnect MQTT");
                                        Serial.println(mqttFailCounter);
                                }
                                mqttFailCounter++;
                        }
                }
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
        char state3[5];
        char msg2[100];

        //      Display on Serial monitor only
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length] = 0;
        Serial.println("");
        //      ##############################

        //      status - via MQTT
        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn) {
                        sprintf(state, "invalid Relay State");
                }
                else if (outputUp_currentState == !RelayOn && outputDown_currentState == RelayOn) {
                        sprintf(state, "DOWN");
                }
                else if (outputUp_currentState == RelayOn && outputDown_currentState == !RelayOn) {
                        sprintf(state, "UP");
                }
                else {
                        sprintf(state, "OFF");
                }

                // switch state
                if (inputUp_lastState == !SwitchOn && inputDown_lastState == !SwitchOn) {
                        sprintf(state2, "OFF");
                }
                else if (inputUp_lastState == SwitchOn && inputDown_lastState == !SwitchOn) {
                        sprintf(state2, "UP");
                }
                else if (inputUp_lastState == !SwitchOn && inputDown_lastState == SwitchOn) {
                        sprintf(state2, "DOWN");
                }

                // MQTT state
                if (outputUp_currentState == !RelayOn && outputDown_currentState == !RelayOn) {
                        sprintf(state3, "OFF");
                }
                else if (outputUp_currentState == RelayOn && outputDown_currentState == !RelayOn) {
                        sprintf(state3, "UP");
                }
                else if (outputUp_currentState == !RelayOn && outputDown_currentState == RelayOn) {
                        sprintf(state3, "DOWN");
                }

                sprintf(msg, "Status: Relay:[%s], Sw:[%s], MQTT[%s]", state, state2, state3);
                pub_msg(msg);
        }
        //      switch commands via MQTT
        else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0) {
                switchIt("MQTT", incoming_msg);
                repostCounter = millis(); // to avoid hardware and MQTT notifications, fire together

                if (strcmp(incoming_msg, "up") == 0 ) { //for hardware reset
                        lastResetPress = millis();
                        detectResetPresses();
                }
        }
        else if (strcmp(incoming_msg, "boot") == 0 ) {
                sprintf(msg, "Boot:[%s],", bootTime);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s]", ver);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ip") == 0 ) {
                char buf[16];
                sprintf(buf, "IP:%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
                sprintf(msg, "IP address:[%s]", buf);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "reset") == 0 ) {
                sendReset("MQTT");
        }
}

void pub_msg(char *inmsg) {
        char tmpmsg[150];

        get_timeStamp();
        sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic );
        msgSplitter(inmsg, 95, tmpmsg, "#" );
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
                        client.publish(msgTopic, tmp);

                }
        }
        else {
                sprintf(tmp, "%s %s", prefix, msg_in);
                client.publish(msgTopic, tmp);
        }
}

void get_timeStamp() {
        time_t t = now();
        sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}

void switchIt(char *type, char *dir) {
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir, "up") == 0) {
                states[0] = RelayOn;
                states[1] = !RelayOn;
        }
        else if (strcmp(dir, "down") == 0) {
                states[0] = !RelayOn;
                states[1] = RelayOn;
        }
        else if (strcmp(dir, "off") == 0) {
                states[0] = !RelayOn;
                states[1] = !RelayOn;
        }

        allOff();
        delay(deBounceInt);
        digitalWrite(outputUpPin, states[0]);
        delay(deBounceInt);
        digitalWrite(outputDownPin, states[1]);

        client.publish(stateTopic, dir, true); // post a Retained State

        sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
        pub_msg(mqttmsg);
}

void detectResetPresses() {
        if (millis() - lastResetPress < timeInt2Reset) {
                Serial.println(millis() - lastResetPress);
                if (manResetCounter >= pressAmount2Reset) {
                        sendReset("Manual operation");
                }
                else {
                        manResetCounter++;
                }
        }
        else {
                manResetCounter = 0;
        }
}

void sendReset(char *header) {
        char temp[150];

        Serial.println("Sending Reset command");
        sprintf(temp, "[%s] - Reset sent", header);
        pub_msg(temp);
        delay(100);
        ESP.restart();
}

void PBit() {
        allOff();
        int t = deBounceInt*5;

        digitalWrite(outputUpPin, RelayOn);
        delay(t);
        digitalWrite(outputUpPin, !RelayOn);
        delay(t);
        digitalWrite(outputDownPin, RelayOn);
        delay(t);

        allOff();

}

void verifyMQTTConnection() {
        //  MQTT reconnection for first time or after first insuccess to reconnect
        if (!client.connected() && firstNotConnected == 0) {
                connectionFlag = connectMQTT();
                //  still not connected
                if (connectionFlag == 0 ) {
                        firstNotConnected = millis();
                }
                else {
                        client.loop();
                }
        }
        // retry after fail - resume only after timeout
        else if (!client.connected() && firstNotConnected != 0 && millis() - firstNotConnected > MQTTtimeOut) {
                //    after cooling out period - try again
                connectionFlag = connectMQTT();
                firstNotConnected = 0;
                Serial.println("trying again to reconnect");
        }
        else {
                client.loop();
        }
}

void allOff() {
        digitalWrite(outputUpPin, !RelayOn);
        digitalWrite(outputDownPin, !RelayOn);
        inputDown_lastState = digitalRead(inputDownPin);
        inputUp_lastState = digitalRead(inputUpPin);
}

void checkSwitch_PressedUp() {
        char mqttmsg[100];

        if (inputUp_currentState != inputUp_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputUpPin) != inputUp_lastState) {
                        sprintf(mqttmsg, "[%s] switched [%s]", "Button", digitalRead(inputUpPin) ? "up" : "off");
                        if (millis() - repostCounter >= MQTTtimeOut_repost) { // to avoid MQTT msg and Button msgs together
                                pub_msg(mqttmsg);
                        }

                        inputUp_lastState = digitalRead(inputUpPin);
                }

        }
}

void checkSwitch_PressedDown() {
        char mqttmsg[100];

        if (digitalRead(inputDownPin) != inputDown_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputDownPin) != inputDown_lastState) {
                        sprintf(mqttmsg, "[%s] switched [%s]", "Button", digitalRead(inputDownPin) ? "down" : "off");
                        if (millis() - repostCounter >= MQTTtimeOut_repost) {
                                pub_msg(mqttmsg);
                        }

                        inputDown_lastState = digitalRead(inputDownPin);
                }
        }

}

void verifyNotHazardState() {
        if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn ) {
                switchIt("Button", "off");
                Serial.println("Hazard state - both switches were ON");
                pub_msg("HazradState - Reset");
                sendReset("Hazard");
        }

}

void takeTheDog() {
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset("WDT");
        }
}

void readGpioStates() {
        outputUp_currentState = digitalRead(outputUpPin);
        outputDown_currentState = digitalRead(outputDownPin);
        inputDown_currentState = digitalRead(inputDownPin);
        inputUp_currentState = digitalRead(inputUpPin);
}

void loop() {
        wdtResetCounter = 0; // reset WDT
        verifyMQTTConnection();
        readGpioStates();
        verifyNotHazardState(); // both up and down are ---> OFF
        checkSwitch_PressedUp();
        checkSwitch_PressedDown();

        delay(50);
}
