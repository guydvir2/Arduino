#include "Arduino.h"
#include "myIOT.h"

#include <ESP8266Ping.h>
#include <myJSON.h>

myJSON json(jfile, true);

// ~~~~~~ myIOT CLASS ~~~~~~~~~~~ //
myIOT::myIOT(char *devTopic, char *key)
        : _failNTPcounter_inFlash(key), mqttClient(espClient)
{
        strcpy(_deviceName,devTopic); // for OTA only
}
void myIOT::start_services(cb_func funct, char *ssid, char *password, char *mqtt_user, char *mqtt_passw, char *mqtt_broker) {
        mqtt_server = mqtt_broker;
        user = mqtt_user;
        passw = mqtt_passw;
        ssid = ssid;
        password = password;
        ext_mqtt = funct;       //redirecting to ex-class function ( defined outside)
        extDefine = true;       // maing sure this ext_func was defined


        if ( useSerial ) {
                Serial.begin(9600);
                delay(10);
        }
        startNetwork(ssid, password);
        if (useWDT) {
                startWDT();
        }
        if (useOTA) {
                startOTA();
        }
}
void myIOT::looper(){
        networkStatus();      // runs wifi/mqtt connectivity
        publish_errs();

        //reset WDT watchDog
        wdtResetCounter = 0;

        // checks for OTA
        if (useOTA) {
                acceptOTA();
        }
        // continue TO after reset
        if(useResetKeeper) {
                if(firstRun && mqtt_detect_reset !=2) {
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
                strcat(bootErrors,"**NoWifi** ");
                noNetwork_Clock = millis(); // CHANGE V1
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
        if (WiFi.status() == WL_CONNECTED ) {   // wifi is ok
                if (mqttClient.connected()) {   // mqtt is good
                        mqttClient.loop();
                        mqttConnected    = 1;
                        noNetwork_Clock  = 0;
                        if ( alternativeMQTTserver ==true) { // connected to temp MQTT
                                if  (millis() - noNetwork_Clock >= time2Reset_noNetwork) {// reset due to long no MQTT
                                        sendReset("fail MQTT");
                                }
                        }
                }
                else if (noNetwork_Clock == 0) { // first time no MQTT, wifi OK
                        noNetwork_Clock  = millis();
                }
                else if (noNetwork_Clock !=0 ) {  // WIFI OK, no MQTT not for the first time
                        if (millis() - noNetwork_Clock >= time2Reset_noNetwork) {     // reset due to long no MQTT
                                sendReset("NO NETWoRK");
                        }
                        else if (millis() - noNetwork_Clock <= time2_tryReconnect) { // time interval to try again to connect MQTT
                                if (subscribeMQTT() == 1) {                           //try successfully reconnect mqtt
                                        noNetwork_Clock  = 0;
                                        mqttConnected    = 1;
                                }
                                else {         // fail connect to MQTT server
                                        mqttConnected = 0;
                                }
                        }
                }
        }
        else {                                  // NO WIFI

                if (noNetwork_Clock == 0) { // first time when NO NETWORK
                        noNetwork_Clock=millis();
                }
                else{
                        if  (millis() - noNetwork_Clock >= time2Reset_noNetwork) {
                                sendReset("NOWIFI"); // due to wifi error
                        }
                }
        }
}
void myIOT::start_clock() {
        int failcount;
        if (startNTP()) {   //NTP Succeed
                _failNTP = false;
                get_timeStamp();
                strcpy(bootTime, timeStamp);
                if(_failNTPcounter_inFlash.getValue(failcount)) {
                        if (failcount!=0) {
                                _failNTPcounter_inFlash.setValue(0);

                        }
                }
                else{
                        _failNTPcounter_inFlash.setValue(0);
                        strcat(bootErrors,"**Fail Write Flash NTP** ");
                }
        }
        else{
                strcat(bootErrors,"** Fail connecting NTP server** ");
                if (resetFailNTP) {
                        if(_failNTPcounter_inFlash.getValue(failcount)) {
                                if (failcount<3) {
                                        _failNTPcounter_inFlash.setValue(failcount+1);
                                        ESP.reset();

                                }
                        }
                        else{
                                _failNTPcounter_inFlash.setValue(1);
                                strcat(bootErrors,"set value in flash");
                        }
                }
                _failNTP = true;
        }

}
bool myIOT::startNTP() {
        byte x=0;
        byte retries = 5;
        int delay_tries = 300;
        char* NTPserver="pool.ntp.org";

        NTP.begin(NTPserver, 2, true);
        delay(delay_tries);
        time_t t=now();

        while (x < retries && year(t)==1970) {
                NTP.begin(NTPserver, 2, true);
                delay(delay_tries*(1.2*(x+1)));
                t=now();
                x+=1;
        }
        if(x<retries && year(t)!=1970) {
                NTP.setInterval(5, clockUpdateInt);
                return 1;
        }
        if (year(t)==1970) {
                strcat(bootErrors,"** NTP Fail obtain valid Clock ** ");
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
        bool stat = false;
        createTopics();
        // Select MQTT server
        if (Ping.ping(mqtt_server)) {
                mqttClient.setServer(mqtt_server, 1883);
                stat = true;
                alternativeMQTTserver  =false;
                if (useSerial) {
                        Serial.println("MQTT SERVER1");
                }
        }
        else if (Ping.ping(mqtt_server2)) {
                mqttClient.setServer(mqtt_server2, 1883);
                strcat(bootErrors,"** MQTT SERVER ERR **");
                if(useSerial) {
                        strcat(bootErrors,"Coonected to MQTT SERVER2");
                }
                alternativeMQTTserver  =false;
                stat = true;
        }
        // Set callback function
        if (stat) {
                mqttClient.setCallback(std::bind(&myIOT::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                subscribeMQTT();
        }
        else {
                if (useSerial) {
                        Serial.println("Not connected to MQTT server");
                }
        }
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
                        if (mqttClient.connect(_deviceName, user, passw, _availTopic, 0, true, "offline")) {
                                if (useSerial) {
                                        Serial.println("connected");
                                }
                                mqttConnected = 1;
                                if (useResetKeeper == false) {
                                        notifyOnline();
                                }
                                if (firstRun == true) {
                                        pub_err("<< Boot >>");
                                        if (useResetKeeper == false) {
                                                firstRun = false;
                                        }
                                }
                                else {
                                        notifyOnline();
                                        // sprintf(msg, "<< Connected to MQTT - Reload [%d]>> ", mqttFailCounter);
                                        // pub_err(msg);

                                }
                                for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++) {
                                        if (strcmp(topicArry[i],"")!=0) {
                                                mqttClient.subscribe(topicArry[i]);
                                        }
                                }

                                mqttFailCounter = 0;
                                return 1;
                        }
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
                strcat(bootErrors,"** NoWifi ** ");
                return 0;
        }
}
void myIOT::createTopics() {
        snprintf(_msgTopic, MaxTopicLength, "%s/Messages", prefixTopic);
        snprintf(_groupTopic, MaxTopicLength, "%s/All", prefixTopic);
        snprintf(_errorTopic, MaxTopicLength, "%s/Errors", prefixTopic);

        if(strcmp(addGroupTopic,"")!=0) {
                char temptopic[MaxTopicLength];
                strcpy(temptopic, addGroupTopic);
                snprintf(addGroupTopic,MaxTopicLength,"%s/%s",prefixTopic,temptopic);

                snprintf(deviceTopic,MaxTopicLength,"%s/%s",addGroupTopic,_deviceName);
        }
        else{
                snprintf(deviceTopic,MaxTopicLength,"%s/%s",prefixTopic,_deviceName);
        }

        snprintf(_stateTopic, MaxTopicLength, "%s/State", deviceTopic);
        snprintf(_availTopic, MaxTopicLength, "%s/Avail", deviceTopic);


}
void myIOT::callback(char* topic, byte* payload, unsigned int length) {
        char incoming_msg[150];
        char state[5];
        char state2[5];
        char msg2[100];

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

        // Check if Avail topic starts from OFFLINE or ONLINE mode
        // This will flag weather unwanted Reset occured
        if (firstRun && useResetKeeper) {
                if(strcmp(topic,_availTopic)==0) {
                        if (strcmp(incoming_msg,"online")==0) {
                                mqtt_detect_reset = 1; // bad reboot
                        }
                        else if (strcmp(incoming_msg,"offline")==0) {
                                mqtt_detect_reset = 0; // ordinary boot
                        }
                }
        }

        if      (strcmp(incoming_msg, "boot") == 0 ) {
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
                allowOTA_clock = millis();
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
                msgSplitter(inmsg, 200, tmpmsg, "#" );
        }
}
void myIOT::pub_state(char *inmsg) {
        if (mqttConnected == true) {
                mqttClient.publish(_stateTopic, inmsg, true);
        }
}
void myIOT::pub_err(char *inmsg) {
        char tmpmsg[250];
        get_timeStamp();
        sprintf(tmpmsg, "[%s] [%s] Error log: %s", timeStamp, deviceTopic, inmsg );

        if (mqttConnected == true) {
                mqttClient.publish(_errorTopic, tmpmsg);
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
                                mqttClient.publish(_msgTopic, tmp);
                        }
                }
        }
        else {
                if (mqttConnected == true) {
                        sprintf(tmp, "%s %s", prefix, msg_in);
                        mqttClient.publish(_msgTopic, tmp);
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
        mqttClient.publish(_availTopic, "online", true);
}
void myIOT::notifyOffline(){
        mqttClient.publish(_availTopic, "offline", true);
}
void myIOT::publish_errs(){
        if (strcmp(bootErrors,"")!=0 &&mqttConnected==true) {
                pub_err(bootErrors);
                strcpy(bootErrors,"");
        }
}

// ~~~~~~ Reset and maintability ~~~~~~
void myIOT::sendReset(char *header) {
        char temp[150];

        if (useSerial) {
                Serial.println("Sending Reset command");
        }
        if (strcmp(header, "null") != 0) {
                sprintf(temp, "[%s] - Reset sent", header);
                Serial.println(temp);
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
        if (millis() - allowOTA_clock <= OTA_upload_interval) {
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

        allowOTA_clock = millis();

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
FVars::FVars(char* key, char* pref){
        sprintf(_key,"%s%s",pref,key);
}
bool FVars::getValue(int &ret_val){
        return json.getValue(_key, ret_val);
}
bool FVars::getValue(long &ret_val){
        return json.getValue(_key, ret_val);
}
bool FVars::getValue(char value[20]){
        return json.getValue(_key, value);
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


// ~~~~~~~~~~~ TimeOut Class ~~~~~~~~~~~~
timeOUT::timeOUT(char* sw_num, int def_val)
        : endTimeOUT_inFlash(_key1,sw_num), inCodeTimeOUT_inFlash(_key2,sw_num), updatedTimeOUT_inFlash(_key3,sw_num)
{
        /* endTimeOUT_inFlash     -- Save clock when TO ends (sec from epoch)
           inCodeTimeOUT_inFlash  -- save value of TO defined in code [ minutes]
           updatedTimeOUT_inFlash -- value of TO defined using MQTT by user, overides inCode value [minutes]
         */

        int tempVal_inFlash  = 0;
        inCodeTO = def_val; //[min]

        if(updatedTimeOUT_inFlash.getValue(updatedTO_inFlash) != true) { // not able to read
                updatedTimeOUT_inFlash.setValue(0);
        }

        if(endTimeOUT_inFlash.getValue(endTO_inFlash) != true) { // not able to read
                endTimeOUT_inFlash.setValue(0);
        }

        if(inCodeTimeOUT_inFlash.getValue(tempVal_inFlash) != true) {
                inCodeTimeOUT_inFlash.setValue(0);
        }
        else{
                if (tempVal_inFlash != inCodeTO) {
                        inCodeTimeOUT_inFlash.setValue(inCodeTO);
                }
        }

        if (updatedTO_inFlash != 0) {
                _calc_TO = updatedTO_inFlash;
                Serial.print("update");
                Serial.print(_calc_TO);

        }
        else {
                _calc_TO = inCodeTO;
                                Serial.print(_calc_TO);
        }
}
bool timeOUT::looper(){
        if (_calc_endTO >now()) {
                return 1;
        }
        else {
                if (_inTO == true) {
                        switchOFF();
                }
                return 0;
        }
}
bool timeOUT::begin(bool newReboot){   // NewReboot come to not case of sporadic reboot

        // ~~~~~~~~ Check if stored end value stil valid ~~~~~~~~~~~~~~
        if (endTO_inFlash > now()) {                 // get saved value- still have to go
                _calc_endTO = endTO_inFlash;           //clock time to stop
                switchON();
                return 1;
        }
        else if (endTO_inFlash >0 && endTO_inFlash <=now()) {          // saved but time passed
                switchOFF();
                return 0;
        }
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        /*
           case below is the main issue: if upon new reboot, after a successfull
           ending of last timeOut ( endTO_inFlash==0 ), how to consider a new
           Reboot ? if newReboot == true, means that it will start over
           as a new Timeout Task.
         */

        // ~~~~~~~~~ Case of fresh Start - not a quick boot ~~~~~~~~~~~~~~~~
        else if (endTO_inFlash == 0 && newReboot == true) {           // fresh start
                if (_calc_TO != 0) {
                        setNewTimeout(_calc_TO);
                        return 1;
                }
                else {
                        _calc_endTO = 0;
                        return 0;
                }
        }
        else if (endTO_inFlash == 0) {          // saved but time passed
                return 0;
        }
}
int timeOUT::remain(){
        if (_inTO == true) {
                return _calc_endTO-now(); //sec
        }
        else {
                return 0;
        }
}
void timeOUT::setNewTimeout(int to, bool mins){
        if (mins==true) {
                _calc_endTO=now()+to*60; // when given in mintes
        }
        else{
                _calc_endTO=now()+to;

        }
        endTimeOUT_inFlash.setValue(_calc_endTO); // store end_to to flash
        switchON();
}
void timeOUT::restart_to(){
        setNewTimeout(_calc_TO);
}
void timeOUT::updateTOinflash(int TO){
        updatedTimeOUT_inFlash.setValue(TO);
        setNewTimeout(TO);
}
void timeOUT::restore_to(){
        updatedTimeOUT_inFlash.setValue(0);
        endTimeOUT_inFlash.setValue(0);
        inCodeTimeOUT_inFlash.setValue(0);
}
void timeOUT::switchON(){
        _inTO = true;
}
void timeOUT::switchOFF(){
        endTimeOUT_inFlash.setValue(0);
        endNow();
        _inTO = false;
}
void timeOUT::endNow(){
        _calc_endTO = now();
}
void timeOUT::convert_epoch2clock(long t1, long t2, char* time_str, char* days_str){
        byte days       = 0;
        byte hours      = 0;
        byte minutes    = 0;
        byte seconds    = 0;

        int sec2minutes = 60;
        int sec2hours   = (sec2minutes * 60);
        int sec2days    = (sec2hours * 24);
        int sec2years   = (sec2days * 365);

        long time_delta = t1-t2;

        days    = (int)(time_delta / sec2days);
        hours   = (int)((time_delta - days * sec2days) / sec2hours);
        minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

        sprintf(days_str, "%02d days", days);
        sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}

myTelegram::myTelegram(char* Bot, char* chatID, char* ssid, char* password) : bot (Bot, client)
{
        sprintf(_bot,Bot);
        sprintf(_chatID,chatID);
        sprintf(_ssid,ssid);
        sprintf(_password,password);
}
void myTelegram::handleNewMessages(int numNewMessages){
        char sendmsg[50];

        for (int i=0; i<numNewMessages; i++) {
                String chat_id = String(bot.messages[i].chat_id);
                String text = bot.messages[i].text;
                String from_name = bot.messages[i].from_name;
                if (from_name == "") from_name = "Guest";

                _ext_func (text,from_name,chat_id, sendmsg);
                bot.sendMessage(chat_id, sendmsg, "");
        }
}
void myTelegram::begin(cb_func2 funct){

        _ext_func = funct; // call to external function outside of clss

        if (WiFi.status() != WL_CONNECTED) {
                WiFi.mode(WIFI_STA);
                WiFi.disconnect();
                delay(100);

                Serial.print("Connecting Wifi: ");
                Serial.println(_ssid);
                WiFi.begin(_ssid, _password);

                while (WiFi.status() != WL_CONNECTED) {
                        Serial.print(".");
                        delay(500);
                }
                WiFi.setAutoReconnect(true);
        }

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        client.setInsecure();
}
void myTelegram::send_msg(char *msg){
        bot.sendMessage(_chatID, msg, "");
}
void myTelegram::looper(){
        if (millis() > _Bot_lasttime + _Bot_mtbs)  {
                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

                while(numNewMessages) {
                        handleNewMessages(numNewMessages);
                        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                }
                _Bot_lasttime = millis();
        }
}
