//change deviceTopic !
//###################################################

#define deviceTopic "HomePi/Dvir/gardenFlowMeter"
const char *ver = "ESP_WDT_OTA_0.1";

//###################################################

// Service flags
bool useWDT = false;
bool useOTA = false;
int networkID = 1;  // 0: HomeNetwork,  1:Xiaomi_D6C8
bool useNetwork = false;
bool useSerial = true;

#define LEDoff HIGH

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
long firstNotConnected = 0; // time stamp of first try
int connectionFlag = 0;
int MQTTretries = 5; // allowed tries to reconnect
// ######################


// time interval parameters
const int clockUpdateInt = 1; // hrs to update clock
const int timeInt2Reset = 1500; // time between consq presses to init RESET cmd
const long MQTTtimeOut = (1000 * 60) * 5; //5 mins stop try to MQTT
const long WIFItimeOut = (1000 * 60) * 2; //2 mins try to connect WiFi
const long OTAtimeOut = (1000*60) * 2; // 2 minute to try OTA
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


//  Pins to Flow_Meter
byte statusLed = 13;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin = 2;
// ######################

float calibrationFactor = 4.5; // pulses per second per litre/minute of flow.
volatile byte pulseCount = 0;
float flowRate = 0;
unsigned int flow_milLiters = 0;
unsigned long total_milLitres = 0;
unsigned long oldTime = 0;

byte currentDay;
byte currentMonth;
unsigned int currentDay_flow=0; //liters
unsigned int lastDay_flow=0; //liters
unsigned int monthly_consumption [12] = {0,0,0,0,0,0,0,0,0,0,0,0}; //liters
const char months []= {'Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'};
unsigned int adHoc_flow =0; //liters
bool adHoc_flag=false;


void setup(){

        startGPIOs();
        selectNetwork();

        if (useSerial) {
                Serial.begin(38400);
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

        time_t t = now();
        currentDay = day(t);
        currentMonth = month(t);

        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void startGPIOs(){
        pinMode(statusLed, OUTPUT);
        pinMode(sensorPin, INPUT);
        digitalWrite(statusLed, LEDoff); // We have an active-low LED attached
        digitalWrite(sensorPin, HIGH);
}

// From here- all functions are copied from other sketched without any changes
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
        long startWifiConnection = 0;
        startWifiConnection = millis();
        WiFi.mode(WIFI_STA); //OTA Added
        WiFi.begin(ssid, password);
        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED && millis() - startWifiConnection < WIFItimeOut) {
                delay(500);
        }
        WiFi.setAutoReconnect(true);

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
                // if (useSerial) {
                //         Serial.println("Start updating " + type);
                // }
                // Serial.end();
        });
        // if (useSerial) { // for debug
        //         ArduinoOTA.onEnd([]() {
        //                 Serial.println("\nEnd");
        //         });
        //         ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        //                 Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        //         });
        //         ArduinoOTA.onError([](ota_error_t error) {
        //                 Serial.printf("Error[%u]: ", error);
        //                 if (error == OTA_AUTH_ERROR) {
        //                         Serial.println("Auth Failed");
        //                 } else if (error == OTA_BEGIN_ERROR) {
        //                         Serial.println("Begin Failed");
        //                 } else if (error == OTA_CONNECT_ERROR) {
        //                         Serial.println("Connect Failed");
        //                 } else if (error == OTA_RECEIVE_ERROR) {
        //                         Serial.println("Receive Failed");
        //                 } else if (error == OTA_END_ERROR) {
        //                         Serial.println("End Failed");
        //                 }
        //         });
        //         // ArduinoOTA.begin();
        //         Serial.println("Ready");
        //         Serial.print("IP address: ");
        //         Serial.println(WiFi.localIP());
        // }

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
                while (!mqttClient.connected() && mqttFailCounter <= MQTTretries) {
                        // Attempt to connect
                        if (mqttClient.connect(deviceName, user, passw, availTopic, 0, true, "offline")) {
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
                                delay(5000);
                                mqttFailCounter++;
                        }
                }
                mqttFailCounter = 0;
                return 0;
        }
        else {
                return 0;
        }
}

void pub_msg(char *inmsg) {
        char tmpmsg[150];

        get_timeStamp();
        sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic );
        msgSplitter(inmsg, 95, tmpmsg, "#" );
}

void createTopics(const char *devTopic, char *state, char *avail) {
        sprintf(state, "%s/State", devTopic);
        sprintf(avail, "%s/Avail", devTopic);
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

void sendReset(char *header) {
        char temp[150];

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
// #############################





void callback(char* topic, byte* payload, unsigned int length) {
        char incoming_msg[50];
        char state[5];
        char state2[5];
        char msg2[100];

        for (int i = 0; i < length; i++) {
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length] = 0;

        if (strcmp(incoming_msg, "boot") == 0 ) {
                sprintf(msg, "Boot:[%s]", bootTime);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s]", ver);
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

        else if (strcmp(incoming_msg, "flow") == 0 ) {
                float f_lit = (float)total_milLitres/1000;
                sprintf(msg, "Total: %.02f liters",f_lit );
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "start_count") == 0 ) {
                adHoc_flag = true;
                adHoc_flow = 0;
                pub_msg("Start counter");
        }
        else if (strcmp(incoming_msg, "stop_count") == 0 ) {
                adHoc_flag = false;
                pub_msg("Stop counter");
        }
        else if (strcmp(incoming_msg, "show_count") == 0 ) {
                sprintf(msg, "Flow count: %.02f liters",adHoc_flow);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "reset_count") == 0 ) {
                adHoc_flow = 0;
                pub_msg("Reset counter");
        }
}

void print_OL_readings(){
        unsigned int frac;
        // Print the flow rate for this second in litres / minute
        Serial.print("Flow rate: ");
        Serial.print(int(flowRate)); // Print the integer part of the variable
        Serial.print("."); // Print the decimal point
        // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
        frac = (flowRate - int(flowRate)) * 10;
        Serial.print(frac, DEC); // Print the fractional part of the variable
        Serial.print("L/min");
        // Print the number of litres flowed in this second
        Serial.print("  Current Liquid Flowing: "); // Output separator
        Serial.print(flow_milLiters);
        Serial.print("mL/Sec");

        // Print the cumulative total of litres flowed since starting
        Serial.print("  Output Liquid Quantity: "); // Output separator
        Serial.print(total_milLitres);
        Serial.println("mL");
}

void pulseCounter(){
        // Increment the pulse counter
        pulseCount++;
}

void measureFlow(){
        if((millis() - oldTime) > 1000 && pulseCount >0) { // calc cycle
                // Disable the interrupt while calculating flow rate
                detachInterrupt(sensorInterrupt);
                flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
                oldTime = millis();
                // convert to millilitres.
                flow_milLiters = (flowRate / 60) * 1000;
                // Add the millilitres passed in this second to the cumulative total
                total_milLitres += flow_milLiters;
                cummDay();
                print_OL_readings();
                // Reset the pulse counter- for next cycle
                pulseCount = 0;

                // Enable the interrupt again now that we've finished sending output
                attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
        }
}

void cummDay(){
        time_t t = now();

        if (day(t)==currentDay) {
                currentDay_flow += flow_milLiters/1000;
        }
        else {
                lastDay_flow = currentDay_flow;
                currentDay_flow = 0;
                currentDay = day(t);
                currentDay_flow += flow_milLiters/1000;
        }

        if (adHoc_flag == true) {
                adHoc_flow += flow_milLiters/1000;
        }

        monthly_consumption[month(t)] = monthly_consumption[month(t)] +flow_milLiters/1000;
        if (useSerial) {
                Serial.print("Current Day:");
                Serial.println(currentDay_flow);

                Serial.print("Months: ");
                for (int i=0; i<=11;i++) {
                        Serial.print(months[i]);
                        Serial.print(": ");
                        Serial.println(monthly_consumption[i]);
                }

                Serial.print("Counter: ");
                Serial.println(adHoc_flow);
        }



}

void loop(){
        measureFlow();

        // Service updates
        verifyMQTTConnection();
        if (useWDT) {
                wdtResetCounter = 0;
        }
        if (useOTA) {
                acceptOTA();
        }
}
