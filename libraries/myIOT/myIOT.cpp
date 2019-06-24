#include "Arduino.h"
#include "myIOT.h"

#include <ESP8266WiFi.h>
#include <NtpClientLib.h>
#include <PubSubClient.h> //MQTT
#include <Ticker.h>       //WDT
#include <myJSON.h>

// OTA libraries
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// #######################

// ~~~~~~~~~ Services ~~~~~~~~~~~
WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker wdt;
myJSON json(jfile, true);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~ myIOT CLASS ~~~~~~~~~~~ //

myIOT::myIOT(char *devTopic) {
        deviceTopic = devTopic;
        deviceName = deviceTopic;
        topicArry[0] = deviceTopic;
}
void myIOT::start_services(cb_func funct, char *ssid, char *password, char *mqtt_user, char *mqtt_passw, char *mqtt_broker) {
        mqtt_server = mqtt_broker;
        user = mqtt_user;
        passw = mqtt_passw;
        ssid = ssid;
        password = password;
        ext_mqtt = funct;     //redirecting to ex-class function ( defined outside)
        extDefine = true;     // maing sure this ext_func was defined


        if ( useSerial ) {
                Serial.begin(9600);
                delay(10);
                Serial.println("\n~~~~~ IOT begins ~~~~~");
        }
        startNetwork(ssid, password);
        if (useWDT) {
                startWDT();
        }
        if (useOTA) {
                startOTA();
        }
        Serial.println("~~~~~ IOT fully loaded ~~~~~");
}
void myIOT::looper(){
        networkStatus();
        if (useOTA) {
                acceptOTA();
        }
        wdtResetCounter = 0;

        if(useResetKeeper) {
                if(firstRun && encounterReset !=2) {
                        notifyOnline();
                        firstRun = false;
                }
        }
}


// ~~~~~~~ Wifi functions ~~~~~~~
void myIOT::startNetwork(char *ssid, char *password) {
        long startWifiConnection = millis();

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
                delay(150);
                if (useSerial) {
                        Serial.print(".");
                }
        }

        // case of no success - restart due to no wifi
        if (WiFi.status() != WL_CONNECTED) {
                if (useSerial) {
                        Serial.println("no wifi detected");
                }
                noNetwork_Counter = millis(); // CHANGE V1
        }

        // if wifi is OK
        else {
                if (useSerial) {
                        Serial.println("");
                        Serial.println("WiFi connected");
                        Serial.print("IP address: ");
                        Serial.println(WiFi.localIP());
                }
                start_clock();
                startMQTT();
        }
}
void myIOT::networkStatus() {
        if (WiFi.status() == WL_CONNECTED ) { // wifi is ok
                if (mqttClient.connected()) { // mqtt is good
                        mqttClient.loop();
                        mqttConnected = 1;
                        noNetwork_Counter = 0;
                        lastReconnectTry = 0;
                }
                else { // WIFI OK, no MQTT
                        if  (millis() - noNetwork_Counter >= time2Reset_noNetwork) { // reset due to long no MQTT
                                sendReset("null");
                        }
                        else if (lastReconnectTry - (long)millis() <= 0) { // time interval to try again to connect MQTT
                                if (subscribeMQTT() == 1) {         //try successfully reconnect mqtt
                                        noNetwork_Counter = 0;
                                        lastReconnectTry = 0;
                                }
                                else {         // fail connect to MQTT server
                                        if (noNetwork_Counter == 0) {         // first time no MQTT  - start timeout counter
                                                noNetwork_Counter = millis();
                                        }
                                        lastReconnectTry = millis() + time2_tryReconnect;
                                        mqttConnected = 0;
                                }
                        }
                }
        }


        else {             // CHANGE V1 - add this condition - NO WIFI
                if (noNetwork_Counter !=0) { // first time when NO NETWORK
                        noNetwork_Counter=millis();
                }
                if  (millis() - noNetwork_Counter >= time2Reset_noNetwork) {
                        sendReset("null"); // due to wifi error
                }
        }
}
void myIOT::start_clock() {
        if (startNTP()) {   //NTP Succeed
                _failNTP = false;
                get_timeStamp(now());
                //
                // if (bootKeeper()) {
                //         if (resetBoot_flag == false) {   // New Boot
                //                 get_timeStamp(now());
                //                 Serial.println("1");
                //         }
                //         else{   // QuickReset
                //                 get_timeStamp(updated_bootTime);
                //                 Serial.println("2");
                //         }
                // }
                // else{
                //         // sendReset("NTP");
                //         get_timeStamp(now());   // fow now .....
                //         Serial.println("3");
                // }
                strcpy(bootTime, timeStamp);
        }
        else{
                _failNTP = true;
                Serial.println("failNTP");
        }
}
bool myIOT::startNTP() {
        byte x=0;
        byte retries = 5;
        int delay_tries = 500;

        NTP.begin("pool.ntp.org", 2, true);
        delay(delay_tries);
        time_t t=now();

        while (x < retries && year(t)==1970) {
                NTP.begin("pool.ntp.org", 2, true);
                delay(delay_tries);
                t=now();
                x+=1;
        }
        if(x<retries) {
                NTP.setInterval(5, clockUpdateInt);
                return 1;
        }
        else {
                return 0;
        }
}

bool myIOT::bootKeeper() {
        int x =0;
        int suc_counter = 0;
        int maxRetries  = 2;
        long clockShift = 0;

        if (json.getValue("bootCalc", _savedBoot_Calc)) {
                suc_counter+=1;
        }
        else {
                json.setValue("bootCalc", 0);
        }
        delay(300);
        if (json.getValue("bootReset", _savedBoot_reset)) {
                suc_counter+=1;
        }
        else {
                json.setValue("bootReset", 0);
        }

        if (suc_counter == 2) {
                long currentBootTime = now();

                while (x<maxRetries) { // verify time is updated
                        if (year(currentBootTime) != 1970) { //NTP update succeeded
                                json.setValue("bootReset", currentBootTime);
                                int tDelta = currentBootTime - _savedBoot_reset;

                                if ( tDelta > resetIntervals ) {
                                        json.setValue("bootCalc", currentBootTime);
                                        updated_bootTime = currentBootTime;           // take clock of current boot
                                        clockShift = 0;
                                        resetBoot_flag = false;
                                        return 1;
                                }
                                else  {
                                        updated_bootTime = _savedBoot_Calc;           // take clock of last boot
                                        clockShift = currentBootTime - updated_bootTime;
                                        resetBoot_flag = true;
                                        return 1;
                                }
                        }
                        else{
                                currentBootTime = now();
                        }
                        x +=1;
                        delay(200);
                }
                if (x==maxRetries) { // fail NTP
                        Serial.println("bootKeeper Fail");
                        return 0;
                }
        }
        else{
                Serial.println("Fail read/Write bootKeeper to flash");
                return 0;
        }
}
void myIOT::get_timeStamp(time_t t) {
        if (t==0) {
                t = now();
        }
        sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
void myIOT::return_clock(char ret_tuple[20]){
        time_t t = now();
        sprintf(ret_tuple, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}
void myIOT::return_date(char ret_tuple[20]){
        time_t t = now();
        sprintf(ret_tuple, "%02d-%02d-%02d", year(t), month(t), day(t));
}


// ~~~~~~~ MQTT functions ~~~~~~~
void myIOT::startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        mqttClient.setServer(mqtt_server, 1883);
        mqttClient.setCallback(std::bind(&myIOT::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        subscribeMQTT();
}
int myIOT::subscribeMQTT() {
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
                                // mqttClient.publish(availTopic, "online", true);
                                if (firstRun == true) {
                                        mqttClient.publish(stateTopic, "off", true);
                                        pub_msg("<< Boot >>");
                                        // firstRun = false;
                                }
                                else {
                                        sprintf(msg, "<< Connected to MQTT - Reload [%d]>> ", mqttFailCounter);
                                        pub_msg(msg);

                                }
                                for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++) {
                                        mqttClient.subscribe(topicArry[i]);
                                        // sprintf(msg, "Subscribed to %s", topicArry[i]);
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

                        delay(500);
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
void myIOT::createTopics(const char *devTopic, char *state, char *avail) {
        sprintf(state, "%s/State", devTopic);
        sprintf(avail, "%s/Avail", devTopic);
}
void myIOT::callback(char* topic, byte* payload, unsigned int length) {
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
        
        // Check if Avail topic starts from OFFLINE or ONLINE mode
        // This will flag weather unwanted Reset occured
        if (firstRun) {
                if(strcmp(topic,availTopic)==0) {
                        if (strcmp(incoming_msg,"online")==0) {
                                encounterReset = 1;
                                // Serial.println("a: Reset detected");
                        }
                        else if (strcmp(incoming_msg,"offline")==0) {
                                encounterReset = 0;
                                // Serial.println("a: Regular Boot - not reset");
                        }
                }
        }

        if (strcmp(incoming_msg, "boot") == 0 ) {
                sprintf(msg, "Boot:[%s]", bootTime);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ip") == 0 ) {
                char buf[16];
                sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
                sprintf(msg, "IP address:[%s]", buf);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ota") == 0 ) {
                sprintf(msg, "OTA allowed for %d seconds", OTA_upload_interval / 1000);
                pub_msg(msg);
                OTAcounter = millis();
        }
        else if (strcmp(incoming_msg, "reset") == 0 ) {
                sendReset("MQTT");
        }
        else {
                if (extDefine) { // if this function was define then:
                        ext_mqtt(incoming_msg);
                }
        }
}
void myIOT::pub_msg(char *inmsg) {
        char tmpmsg[250];
        get_timeStamp();

        // if (useSerial==true) {
        //         sprintf(tmpmsg, "[%s] [%s] %s", timeStamp, deviceTopic, inmsg );
        //         Serial.println(tmpmsg);
        // }
        // else if (mqttConnected == true) {
        if (mqttConnected == true) {
                sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic );
                msgSplitter(inmsg, 100, tmpmsg, "#" );
        }
}
void myIOT::pub_state(char *inmsg) {
        if (mqttConnected == true) {
                mqttClient.publish(stateTopic, inmsg, true);
        }
}
void myIOT::pub_err(char *inmsg) {
        char tmpmsg[150];
        get_timeStamp();
        sprintf(tmpmsg, "[%s] [%s] [Error log:]%s", timeStamp, deviceTopic, inmsg );
        if (mqttConnected == true) {
                mqttClient.publish(errorTopic, tmpmsg);
        }
}
void myIOT::msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg) {
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
                        if (mqttConnected == true) {
                                mqttClient.publish(msgTopic, tmp);
                        }
                }
        }
        else {
                if (mqttConnected == true) {
                        sprintf(tmp, "%s %s", prefix, msg_in);
                        mqttClient.publish(msgTopic, tmp);
                }
        }
}
int myIOT::inline_read(char *inputstr) {
        char * pch;
        int i = 0;

        pch = strtok (inputstr, " ,.-");
        while (pch != NULL)
        {
                sprintf(inline_param[i], "%s", pch);
                pch = strtok (NULL, " ,.-");
                i++;
        }
        return i;
}
void myIOT::notifyOnline(){
        mqttClient.publish(availTopic, "online", true);
}

// ~~~~~~ Reset and maintability ~~~~~~
void myIOT::sendReset(char *header) {
        char temp[150];

        if (useSerial) {
                Serial.println("Sending Reset command");
        }
        if (strcmp(header, "null") != 0) {
                sprintf(temp, "[%s] - Reset sent", header);
                pub_msg(temp);
        }
        delay(1000);
        ESP.reset();
}
void myIOT::feedTheDog() {
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset("Dog goes woof");
        }
}
void myIOT::acceptOTA() {
        if (millis() - OTAcounter <= OTA_upload_interval) {
                ArduinoOTA.handle();
        }
}
void myIOT::startOTA() {
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
                //    if (useSerial) {
                // Serial.println("Start updating " + type);
                //    }
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
                // Serial.println("Ready");
                // Serial.print("IP address: ");
                // Serial.println(WiFi.localIP());
        }

        ArduinoOTA.begin();
}
void myIOT::startWDT() {
        wdt.attach(1, std::bind(&myIOT::feedTheDog, this)); // Start WatchDog
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~ FVars CLASS ~~~~~~~~~~~ //
FVars::FVars(char* key){
        _key=key;
        int int_value;
        long long_value;
        char char_value[20];

}
bool FVars::getValue(int &ret_val){
        json.getValue(_key, ret_val);
}
bool FVars::getValue(long &ret_val){
        json.getValue(_key, ret_val);
}
bool FVars::getValue(char value[20]){
        json.getValue(_key, value);
}
void FVars::setValue(int val){
        json.setValue(_key, val);
}
void FVars::setValue(long val){
        json.setValue(_key, val);
}
void FVars::setValue(char *val){
        json.setValue(_key, val);
}

void FVars::remove(){
        json.removeValue(_key);
}
void FVars::printFile(){
        json.printFile();
}
void FVars::format(){
        json.format();
}
