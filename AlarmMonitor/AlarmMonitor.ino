//change deviceTopic !
//###################################################

#define deviceTopic "HomePi/Dvir/alarmMonitor"

// Service flags
bool useNetwork = true;
bool useWDT = true;
bool useSerial = false;
bool useOTA = true;

const char *ver = "ESP_WDT_OTA_1.2";

//###################################################

#define RelayOn HIGH
#define SwitchOn LOW

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
const int systemState_armed_Pin = 4;
const int systemState_alarm_Pin = 5;
const int armedHomePin = 12;
const int armedAwayPin = 14;
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
bool armedHome_currentState;
bool armedAway_currentState;
bool systemState_armed_lastState;
bool systemState_alarm_lastState;
bool systemState_armed_currentState;
bool systemState_alarm_currentState;
// ###########################


// time interval parameters
const int clockUpdateInt = 1; // hrs to update clock
const int timeInt2Reset = 1500; // time between consq presses to init RESET cmd
const long MQTTtimeOut = (1000 * 60) * 5; //5 mins stop try to MQTT
const long WIFItimeOut = (1000 * 60) * 2; //2 mins try to connect WiFi
const long OTAtimeOut = (1000*60) * 2; // 2 minute to try OTA
long OTAcounter =0;
const int deBounceInt = 50; //
const int systemPause = 1000; // seconds, delay to system react
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
                Serial.println("AlarmSystem Boot");
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
}

void startGPIOs() {
        pinMode(systemState_armed_Pin, INPUT_PULLUP);
        pinMode(systemState_alarm_Pin, INPUT_PULLUP);
        pinMode(armedHomePin, OUTPUT);
        pinMode(armedAwayPin, OUTPUT);
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
                                        mqttClient.publish(stateTopic, "disarmed", true);
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
                if (digitalRead(armedHomePin) == RelayOn && digitalRead(armedAwayPin) == RelayOn) {
                        sprintf(state, "invalid [Armed] and [Away] State");
                }
                else if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn && digitalRead(systemState_armed_Pin) == SwitchOn) {
                        sprintf(state, "[Armed] Manual");
                }
                else if (digitalRead(armedHomePin) == RelayOn && digitalRead(armedAwayPin) == !RelayOn && digitalRead(systemState_armed_Pin) == SwitchOn) {
                        sprintf(state, "[Home Armed]");
                }
                else if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == RelayOn && digitalRead(systemState_armed_Pin) == SwitchOn) {
                        sprintf(state, "[Away Armed]");
                }
                else if (digitalRead(systemState_armed_Pin) == SwitchOn && digitalRead(systemState_alarm_Pin)== SwitchOn) {
                        sprintf(state, "[Alarm]");
                }
                else if (digitalRead(systemState_armed_Pin) == !SwitchOn && digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                        sprintf(state, "[disarmed]");
                }
                else {
                        sprintf(state, "[notDefined]");

                }

                pub_msg(state);
        }
        else if (strcmp(incoming_msg, "armed_home") == 0 || strcmp(incoming_msg, "armed_away") == 0 || strcmp(incoming_msg, "disarmed") == 0) {
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
                sprintf(msg, "Switch: input1[%d] input2[%d], Relay: output_home[%d] output_full[%d]", systemState_armed_Pin, systemState_alarm_Pin, armedHomePin, armedAwayPin);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ip") == 0 ) {
                char buf[16];
                sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
                sprintf(msg, "IP address:[%s]", buf);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ota") == 0 ) {
                sprintf(msg, "OTA allowed for %d seconds", OTAtimeOut/1000);
                pub_msg(msg);
                OTAcounter = millis();
        }
        else if (strcmp(incoming_msg, "reset") == 0 ) {
                sendReset("MQTT");
        }
        else if (strcmp(incoming_msg, "clear") == 0 ) {
                allReset();
                sendReset("clear");
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
        bool flag=false;

        if (strcmp(dir, "armed_home") == 0 ) {
                if (digitalRead(armedHomePin) == !RelayOn) { // verify it is not in desired state already
                        if ( digitalRead(armedAwayPin) == RelayOn) { // in armed away state
                                digitalWrite(armedAwayPin, !RelayOn);
                                pub_msg("[Code] [Disarmed Away]");
                                delay(systemPause);
                        }

                        digitalWrite(armedHomePin, RelayOn);         // Now switch to armed_home
                        delay(systemPause*4);

                        if (digitalRead(systemState_armed_Pin)==SwitchOn) {
                                pub_msg("[Code] [Armed Home]");
                                flag=true;
                        }
                        else {
                                // allReset();
                                pub_msg("failed to [Armed Home]. [Code] [Disarmed]");
                        }


                }
                else {
                        pub_msg("already in [Armed Home]");
                }
        }

        else if (strcmp(dir, "armed_away") == 0) {
                if ( digitalRead(armedAwayPin) == !RelayOn) {
                        if ( digitalRead(armedHomePin) == RelayOn) { // armed home
                                digitalWrite(armedHomePin, !RelayOn);
                                pub_msg("[Code] [Disarmed Home]");
                                delay(systemPause);
                        }

                        digitalWrite(armedAwayPin, RelayOn); // now switch to Away
                        delay(systemPause*2);

                        if (digitalRead(systemState_armed_Pin)==SwitchOn) {
                                pub_msg("[Code] [Armed Away]");
                                flag=true;
                        }
                        else {
                                // allReset();
                                pub_msg("failed to [Away Home]. [Code] [Disarmed]");
                        }
                }
                else {
                        pub_msg("already in [Away Home]");
                }
        }

        else if (strcmp(dir, "disarmed") == 0) {
                if (systemState_armed_lastState == SwitchOn) { // system is armed
                        allOff();
                        delay(systemPause*4);
                        if (digitalRead(systemState_armed_Pin)==!SwitchOn && digitalRead(armedAwayPin)==!RelayOn && digitalRead(armedHomePin)==!RelayOn) {
                                pub_msg("[Code] [Disarmed]");
                                flag=true;
                        }
                }
        }

        if (useNetwork == true) {
                if (flag == true) {
                        mqttClient.publish(stateTopic, dir, true);
                }
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

void allOff() {
        digitalWrite(armedHomePin, !RelayOn);
        digitalWrite(armedAwayPin, !RelayOn);
        delay(systemPause);
        readGpioStates();
}

void allReset() {
        digitalWrite(armedHomePin, RelayOn);
        delay(systemPause);
        digitalWrite(armedHomePin, !RelayOn);
        delay(systemPause);
        digitalWrite(armedAwayPin, RelayOn);
        delay(systemPause);
        digitalWrite(armedAwayPin, !RelayOn);
        delay(systemPause);

        readGpioStates();
}

void check_systemState_armed() {
        bool temp_systemState_armed_Pin = digitalRead(systemState_armed_Pin);

        if (temp_systemState_armed_Pin != systemState_armed_lastState) {
                delay(deBounceInt);
                if (digitalRead(systemState_armed_Pin) != systemState_armed_lastState) {
                        delay(systemPause);

                        if (temp_systemState_armed_Pin == SwitchOn) { // system is set to armed
                                if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                                        pub_msg("[Manual] [Armed]");
                                }
                                else if (digitalRead(armedAwayPin) == RelayOn || digitalRead(armedHomePin) == RelayOn) {
                                        pub_msg("[Code] [Armed]");
                                }
                        }

                        else { // system Disarmed
                                if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                                        pub_msg("[Disarmed]");
                                }
                                else {
                                        allOff();
                                        if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                                                pub_msg("after retry- [Code] [Disarmed]");
                                        }
                                        else {
                                                pub_msg("failed to [Disarm]");
                                        }
                                }


                        }
                        systemState_armed_lastState = digitalRead(systemState_armed_Pin);
                }
        }
}

void check_systemState_alarming() {
        bool temp_systemState_alarm_Pin = digitalRead(systemState_alarm_Pin);

        if (temp_systemState_alarm_Pin != systemState_alarm_lastState) {
                delay(deBounceInt);
                if (digitalRead(systemState_alarm_Pin) != systemState_alarm_lastState) {
                        delay(systemPause);

                        if (digitalRead(systemState_alarm_Pin) == SwitchOn) {
                                pub_msg("System is [Alarming!]");
                        }
                        else if (digitalRead(systemState_alarm_Pin) == !SwitchOn) {
                                pub_msg("System stopped [Alarming]");
                        }
                        systemState_alarm_lastState = digitalRead(systemState_alarm_Pin);
                }
        }
}

void verifyNotHazardState() {
        if (digitalRead(armedHomePin) == RelayOn && digitalRead(armedAwayPin) == RelayOn ) {
                switchIt("MQTT", "disarmed");
                if (useSerial) {
                        Serial.println("Hazard state - Home and Away states were ON");
                }
                sendReset("HazradState_1");
        }
        if (digitalRead(systemState_armed_Pin) == !SwitchOn) {
                delay(systemPause*5);
                if (digitalRead(armedHomePin) == RelayOn || digitalRead(armedAwayPin) == RelayOn) {
                        if (useSerial) {
                                Serial.println("Hazard state - System Not armed, but states are switched on");
                        }
                        sendReset("HazradState_2");

                }
        }

}

void readGpioStates() {
        armedHome_currentState = digitalRead(armedHomePin);
        armedAway_currentState = digitalRead(armedAwayPin);
        systemState_alarm_currentState = digitalRead(systemState_alarm_Pin);
        systemState_armed_currentState = digitalRead(systemState_armed_Pin);
}

void loop() {
        // read GPIOs
        readGpioStates();
        // verifyNotHazardState();

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
        check_systemState_armed();
        check_systemState_alarming();

        delay(50);
}
