//change deviceTopic !
//###################################################

#define deviceTopic "HomePi/Dvir/flowMeter"

const char *ver = "ESP_WDT_OTA_0.2";
//###################################################

// Service flags
bool useNetwork = true;
bool useWDT = true;
bool useSerial = true;
bool useOTA = true;
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
//  ~~~~~Pins to Flow_Meter
byte statusLed = 13;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin = 2;
// ######################

// ~~~~~ OnLine measuring ~~~~~~~
float calibrationFactor = 4.5; // pulses per second per litre/minute of flow.
volatile byte pulseCount = 0;
float flowRate = 0;
int flow_milLiters = 0;
long total_milLitres = 0;
long oldTime = 0;

// ~~~~~ MQTT messaging ~~~~~~~
char* systemStates[] = {"idle", "flowing"};


// ~~~~~ Cumalatibe consumption
byte currentDay;
byte currentMonth;
float currentDay_flow=0; //liters
float lastDay_flow=0; //liters
float monthly_consumption [12] = {0,0,0,0,0,0,0,0,0,0,0,0}; //liters
const char *months []= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
float adHoc_flow =0; //liters
bool adHoc_flag=false;
bool lastDetectState=false;


// device state definitions
#define LEDoff HIGH


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

        // ~~~ time on Boot ~~~~~
        time_t t = now();
        currentDay = day(t);
        currentMonth = month(t)-1;

        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void startGPIOs(){
        pinMode(sensorPin, INPUT);
        digitalWrite(statusLed, LEDoff); // We have an active-low LED attached
        digitalWrite(sensorPin, HIGH);
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
                // noNetwork_Counter = 0;
                mqttConnected = 1;
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


//  ~~~~ MQTT messages ~~~~~~~
void pub_state(char *inmsg){
        if (mqttConnected ==true) {
                mqttClient.publish(stateTopic, inmsg, true);
        }
}
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
                sprintf(msg, "OTA allowed for %d seconds", OTA_upload_interval/1000);
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
void notifyFlow_state(){
        int threshold = 10;
        if (flowRate > (float)threshold) {
                if(mqttConnected == 1 && lastDetectState != true) {
                        pub_state(systemStates[1]);
                        lastDetectState = true;
                }
        }
        else {
                if(mqttConnected == 1 && lastDetectState == true) {
                        pub_state (systemStates[0]);
                        lastDetectState = false;
                }
        }
}
void print_OL_readings(){
        if (useSerial == true) {
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
}

// ~~~~~~~Measure flow ~~~~~~~~
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

                totalFlow_counter();
                print_OL_readings();

                // Reset the pulse counter- for next cycle
                pulseCount = 0;

                // Enable the interrupt again now that we've finished sending output
                attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
        }
}
void totalFlow_counter(){
        time_t t = now();

        // Day use
        if (day(t)==currentDay) {
                currentDay_flow += (float)flow_milLiters/1000;
        }
        else {
                lastDay_flow = (float)currentDay_flow;
                currentDay_flow = (float)flow_milLiters/1000;
                currentDay = day(t);
        }

        // monthly use
        if (month(t) == currentMonth) {
                monthly_consumption[month(t)-1] = (float)monthly_consumption[month(t)-1] +(float)flow_milLiters/1000;
        }
        else{
                monthly_consumption[month(t)-1] = (float)flow_milLiters/1000;
                currentMonth = month(t);
        }

        // Counter use
        if (adHoc_flag == true) {
                adHoc_flow += (float)flow_milLiters/1000;
        }

        // Report
        if (useSerial) {
                Serial.print("Current Day:");
                Serial.println(currentDay_flow);

                for (int i=0; i<=11; i++) {
                        Serial.print(months[i]);
                        Serial.print(": ");
                        Serial.print(monthly_consumption[i]);
                        Serial.println(" [liters]");
                }

                Serial.print("Counter: ");
                Serial.println(adHoc_flow);
        }
}


void loop(){
        measureFlow();

        // Service updates
        if (useNetwork) {
                network_check();
                notifyFlow_state();
        }
        if (useWDT) {
                wdtResetCounter = 0;
        }
        if (useOTA) {
                acceptOTA();
        }

}
