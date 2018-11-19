//change deviceTopic !
//###################################################

#define deviceTopic "HomePi/Dvir/alarmMonit"

// Service flags
bool useNetwork = true;
bool useWDT = true;
bool useSerial = false;
bool useOTA = true;
bool runPbit = true;

const char *ver = "ESP_0.1";

//###################################################

#define RelayOn LOW
// #define SwitchOn LOW

#include <Arduino.h>
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

// GPIO Pins for ESP8266
const int armStatus = 4;
const int alarmingStatus = 5;
const int command_armedAway = 14;
const int command_armedHome = 12;
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
long firstNotConnected = 0; // time stamp of first try
int connectionFlag = 0;
int MQTTretries = 5; // allowed tries to reconnect
// ######################


// GPIO status flags
bool outputUp_currentState;
bool outputDown_currentState;
bool armStatus_lastState;
bool alarmingStatus_lastState;
bool armStatus_currentState;
bool alarmingStatus_currentState;
// ###########################


// time interval parameters
const int clockUpdateInt = 1; // hrs to update clock
const int timeInt2Reset = 1500; // time between consq presses to init RESET cmd
const long MQTTtimeOut = (1000 * 60) * 5; //5 mins stop try to MQTT
const long WIFItimeOut = (1000 * 60) * 2; //2 mins try to connect WiFi
const long OTAtimeOut = (1000*60) * 1; // 1 minute to try OTA
long OTAcounter =0;
const int deBounceInt = 50; //
volatile int wdtResetCounter = 0;
const int wdtMaxRetries = 10; //seconds to bITE
// ############################


// RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 3; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
long resetTimer = 0;
// ####################


// assorted
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
// ###################


WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker wdt;

void setup() {
        startGPIOs();
        if (useSerial) {
                Serial.begin(9600);
                Serial.println("SystemBoot");
                delay(10);
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

void startGPIOs() {
        pinMode(armStatus, INPUT_PULLUP);
        pinMode(alarmingStatus, INPUT_PULLUP);
        pinMode(commandAway, OUTPUT);
        pinMode(commandHome, OUTPUT);

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
        WiFi.mode(WIFI_STA); //OTA Added
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

void startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        mqttClient.setServer(mqtt_server, 1883);
        mqttClient.setCallback(callback);
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
                while (!mqttClient.connected() && mqttFailCounter <= MQTTretries) {
                        if (useSerial) {
                                Serial.print("Attempting MQTT connection...");
                        }
                        // Attempt to connect
                        if (mqttClient.connect(deviceName, user, passw, availTopic, 0, true, "offline")) {
                                if (useSerial) {
                                        Serial.println("connected");
                                }
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
                        else {
                                if (useSerial) {
                                        Serial.print("failed, rc=");
                                        Serial.print(mqttClient.state());
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
        char msg2[100];

        //      Display on Serial monitor only
        if (useSerial) {
                Serial.print("Message arrived [");
                Serial.print(topic);
                Serial.print("] ");
        }
        for (int i = 0; i < length; i++) {
                if (useSerial) {
                        Serial.print((char)payload[i]);
                }
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length] = 0;
        if (useSerial) {
                Serial.println("");
        }
        //      ##############################

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
                if (armStatus_lastState == !RelayOn && alarmingStatus_lastState == !RelayOn) {
                        sprintf(state2, "OFF");
                }
                else if (armStatus_lastState == RelayOn && alarmingStatus_lastState == !RelayOn) {
                        sprintf(state2, "UP");
                }
                else if (armStatus_lastState == !RelayOn && alarmingStatus_lastState == RelayOn) {
                        sprintf(state2, "DOWN");
                }
                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "boot") == 0 ) {
                sprintf(msg, "Boot:[%s]", bootTime);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s]", ver);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: Up[%d] Down[%d], Relay: Up[%d] Down[%d]", armStatus, alarmingStatus, commandAway, commandHome);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ip") == 0 ) {
                char buf[16];
                sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
                sprintf(msg, "IP address:[%s]", buf);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "ota") == 0 ) {
                pub_msg("OTA allowed for 60 seconds");
                OTAcounter = millis();
        }
        else if (strcmp(incoming_msg, "reset") == 0 ) {
                sendReset("MQTT");
        }
}

void pub_msg(char *inmsg) {
        char tmpmsg[150];

        if (useNetwork == true) {
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
                        mqttClient.publish(msgTopic, tmp);
                }
        }
        else {
                sprintf(tmp, "%s %s", prefix, msg_in);
                mqttClient.publish(msgTopic, tmp);
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
        if (strcmp(dir, "home") == 0) {
                states[0] = RelayOn;
                states[1] = !RelayOn;
        }
        else if (strcmp(dir, "away") == 0) {
                states[0] = !RelayOn;
                states[1] = RelayOn;
        }
        else if (strcmp(dir, "off") == 0) {
                states[0] = !RelayOn;
                states[1] = !RelayOn;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (outputUp_currentState != states[0] && outputDown_currentState != states[1]) {
                allOff();

                delay(deBounceInt * 2);
                digitalWrite(commandAway, states[0]);
                digitalWrite(commandHome, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (outputUp_currentState != states[0] || outputDown_currentState != states[1]) {
                digitalWrite(commandAway, states[0]);
                digitalWrite(commandHome, states[1]);
        }
        if (useNetwork == true) {
                mqttClient.publish(stateTopic, dir, true);
                sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
                pub_msg(mqttmsg);
        }
        if(useSerial == true) {
                Serial.println(dir);
        }
}

void detectResetPresses() {
        if (millis() - lastResetPress < timeInt2Reset) {
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

void sendReset(char *header) {
        char temp[150];

        if (useSerial) {
                Serial.println("Sending Reset command");
        }
        sprintf(temp, "[%s] - Reset sent", header);
        pub_msg(temp);
        delay(100);
        ESP.restart();
}

void PBit() {
        int pause = 2 * 5 * deBounceInt;
        allOff();
        delay(pause);
        digitalWrite(commandAway, RelayOn);
        delay(pause);
        digitalWrite(commandAway, !RelayOn);
        delay(pause);
        digitalWrite(commandHome, RelayOn);
        delay(pause);
        allOff();

}

void verifyMQTTConnection() {
        //  MQTT reconnection for first time or after first insuccess to reconnect
        if (!mqttClient.connected() && firstNotConnected == 0) {
                connectionFlag = connectMQTT();
                //  still not connected
                if (connectionFlag == 0 ) {
                        firstNotConnected = millis();
                }
                else {
                        mqttClient.loop();
                }
        }
        // retry after fail - resume only after timeout
        else if (!mqttClient.connected() && firstNotConnected != 0 && millis() - firstNotConnected > MQTTtimeOut) {
                //    after cooling out period - try again
                connectionFlag = connectMQTT();
                firstNotConnected = 0;
                if (useSerial) {
                        Serial.println("trying again to reconnect");
                }
        }
        else {
                mqttClient.loop();
        }
}

void allOff() {
        digitalWrite(commandAway, !RelayOn);
        digitalWrite(commandHome, !RelayOn);
        armStatus_lastState = digitalRead(armStatus);
        alarmingStatus_lastState = digitalRead(alarmingStatus);
}

void checkSwitch_PressedUp() {
        bool temp_armStatus = digitalRead(armStatus);

        if (temp_armStatus != armStatus_lastState) {
                delay(deBounceInt);
                if (digitalRead(armStatus) != armStatus_lastState) {
                        if (digitalRead(armStatus) == SwitchOn) {
                                switchIt("Button", "up");
                                armStatus_lastState = digitalRead(armStatus);

                                detectResetPresses();
                                lastResetPress = millis();
                        }
                        else if (digitalRead(armStatus) == !SwitchOn) {
                                switchIt("Button", "off");
                                armStatus_lastState = digitalRead(armStatus);
                        }
                }

                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "UP Bounce: cRead [%d] lRead[%d]", temp_armStatus, armStatus_lastState);
                        pub_msg(tMsg);
                }
        }

}

void checkSwitch_PressedDown() {
        bool temp_alarmingStatus = digitalRead(alarmingStatus);

        if (temp_alarmingStatus != alarmingStatus_lastState) {
                delay(deBounceInt);
                if (digitalRead(alarmingStatus) != alarmingStatus_lastState) {

                        if (digitalRead(alarmingStatus) == SwitchOn) {
                                switchIt("Button", "down");
                                alarmingStatus_lastState = digitalRead(alarmingStatus);
                        }
                        else if (digitalRead(alarmingStatus) == !SwitchOn) {
                                switchIt("Button", "off");
                                alarmingStatus_lastState = digitalRead(alarmingStatus);
                        }
                }
                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "Down Bounce: cRead[%d] lRead[%d]", temp_alarmingStatus, alarmingStatus_lastState);
                        pub_msg(tMsg);
                }
        }
}

void verifyNotHazardState() {
        if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn ) {
                switchIt("Button", "off");
                if (useSerial) {
                        Serial.println("Hazard state - both switches were ON");
                }
                sendReset("HazradState");
        }

}

void feedTheDog() {
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset("WatchDog");
        }
}

void acceptOTA() {
  if (millis() - OTAcounter <= OTAtimeOut) {
    ArduinoOTA.handle();
  }
}

void readGpioStates() {
        outputUp_currentState = digitalRead(commandAway);
        outputDown_currentState = digitalRead(commandHome);
        alarmingStatus_currentState = digitalRead(alarmingStatus);
        armStatus_currentState = digitalRead(armStatus);
}

void loop() {
        // read GPIOs
        readGpioStates();
        verifyNotHazardState(); // both up and down are ---> OFF

        // Service updates
        if (useNetwork) {
                verifyMQTTConnection();
        }
        if (useWDT) {
                wdtResetCounter = 0;
        }
        if (useOTA) {
                acceptOTA();
        }

        // react to commands (MQTT or local switch)
        checkSwitch_PressedUp();
        checkSwitch_PressedDown();

        delay(50);
}