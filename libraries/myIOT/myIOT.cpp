#include "Arduino.h"
#include "myIOT.h"

#include <ESP8266Ping.h>
#include <myJSON.h>

#include <TimeLib.h>
#include <TimeAlarms.h>

myJSON json(jfile, true);

// ~~~~~~ myIOT CLASS ~~~~~~~~~~~ //
myIOT::myIOT(char *devTopic, char *key)
    : _failNTPcounter_inFlash(key), mqttClient(espClient)
{
        strcpy(_deviceName, devTopic); // for OTA only
}
void myIOT::start_services(cb_func funct, char *ssid, char *password, char *mqtt_user, char *mqtt_passw, char *mqtt_broker)
{
        mqtt_server = mqtt_broker;
        user = mqtt_user;
        passw = mqtt_passw;
        Ssid = ssid;
        Password = password;
        ext_mqtt = funct; //redirecting to ex-class function ( defined outside)
        extDefine = true; // maing sure this ext_func was defined

        if (useSerial)
        {
                Serial.begin(9600);
                delay(10);
        }
        start_network_services();
        if (useWDT)
        {
                startWDT();
        }
        if (useOTA)
        {
                startOTA();
        }
}
void myIOT::looper()
{
        if (!noNetwork_flag)
        {                         // if boot start without wifi - only a timeout reset will start with flag is false
                network_looper(); // runs wifi/mqtt connectivity
        }
        wdtResetCounter = 0; //reset WDT watchDog
        if (useOTA)
        { // checks for OTA
                acceptOTA();
        }
        if (noNetwork_Clock > 0)
        { // no Wifi or no MQTT will cause a reset
                if (millis() - noNetwork_Clock > time2Reset_noNetwork)
                {
                        sendReset("NO NETWoRK");
                }
        }
        if (alternativeMQTTserver == true)
        {
                if (millis() > time2Reset_noNetwork)
                {
                        sendReset("Reset- restore main MQTT server");
                }
        }
}

// ~~~~~~~ Wifi functions ~~~~~~~
bool myIOT::startWifi(char *ssid, char *password)
{
        long startWifiConnection = millis();

        if (useSerial)
        {
                Serial.println();
                Serial.print("Connecting to ");
                Serial.println(ssid);
        }
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        // WiFi.setAutoReconnect(true);

        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED && millis() - startWifiConnection < WIFItimeOut)
        {
                delay(500);
                if (useSerial)
                {
                        Serial.print(".");
                }
        }

        // case of no success - restart due to no wifi
        if (WiFi.status() != WL_CONNECTED)
        {
                if (useSerial)
                {
                        Serial.println("no wifi detected");
                }
                noNetwork_Clock = millis();
                return 0;
        }

        // if wifi is OK
        else
        {
                if (useSerial)
                {
                        Serial.println("");
                        Serial.println("WiFi connected");
                        Serial.print("IP address: ");
                        Serial.println(WiFi.localIP());
                }
                return 1;
        }
}
void myIOT::start_network_services()
{
        if (startWifi(Ssid, Password))
        {
                start_clock();
                startMQTT();
        }
        else
        {
                noNetwork_flag = true;
        }
}
void myIOT::network_looper()
{
        if (WiFi.status() == WL_CONNECTED)
        { // wifi is ok
                if (mqttClient.connected())
                { // mqtt is good
                        mqttClient.loop();
                }
                else
                { // not connected mqtt
                        if (subscribeMQTT())
                        { // succeed too reconnect
                                mqttClient.loop();
                                noNetwork_Clock = 0;
                        }
                        else
                        { // failed to reconnect
                                if (noNetwork_Clock == 0)
                                { // first time fail MQTT
                                        noNetwork_Clock = millis();
                                }
                        }
                }
        }
        else
        { // no WIFI
                if (!startWifi(Ssid, Password))
                { // failed to reconnect ?
                        if (noNetwork_Clock == 0)
                        { // first time when NO NETWORK ?
                                noNetwork_Clock = millis();
                        }
                }
                else
                { // reconnect succeeded
                        noNetwork_Clock = 0;
                }
        }
}
void myIOT::start_clock()
{
        int failcount;
        if (startNTP())
        { //NTP Succeed
                _failNTP = false;
                get_timeStamp();
                strcpy(bootTime, timeStamp);
                if (_failNTPcounter_inFlash.getValue(failcount))
                {
                        if (failcount != 0)
                        {
                                _failNTPcounter_inFlash.setValue(0);
                        }
                }
                else
                {
                        _failNTPcounter_inFlash.setValue(0);
                        register_err("Fail Write Flash NTP");
                }
        }
        else
        {
                register_err("Fail connecting NTP server");
                if (resetFailNTP)
                {
                        if (_failNTPcounter_inFlash.getValue(failcount))
                        {
                                if (failcount < 3)
                                {
                                        _failNTPcounter_inFlash.setValue(failcount + 1);
                                        ESP.reset();
                                }
                        }
                        else
                        {
                                _failNTPcounter_inFlash.setValue(1);
                                register_err("set value in flash");
                        }
                }
                _failNTP = true;
        }
}
bool myIOT::startNTP()
{
        byte x = 0;
        byte retries = 5;
        int delay_tries = 300;
        char *NTPserver = "pool.ntp.org";

        NTP.begin(NTPserver, 2, true);
        delay(delay_tries);
        time_t t = now();

        while (x < retries && year(t) == 1970)
        {
                NTP.begin(NTPserver, 2, true);
                delay(delay_tries * (1.2 * (x + 1)));
                t = now();
                x += 1;
        }
        if (x < retries && year(t) != 1970)
        {
                NTP.setInterval(5, clockUpdateInt);
                NTP_OK = true;
                return 1;
        }
        if (year(t) == 1970)
        {
                register_err("NTP Fail obtain valid Clock");
                return 0;
        }
}
void myIOT::get_timeStamp(time_t t)
{
        if (t == 0)
        {
                t = now();
        }
        sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
void myIOT::return_clock(char ret_tuple[20])
{
        time_t t = now();
        sprintf(ret_tuple, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}
void myIOT::return_date(char ret_tuple[20])
{
        time_t t = now();
        sprintf(ret_tuple, "%02d-%02d-%02d", year(t), month(t), day(t));
}
bool myIOT::checkInternet(char *externalSite, byte pings)
{
        return Ping.ping(externalSite, pings);
}

// ~~~~~~~ MQTT functions ~~~~~~~
void myIOT::startMQTT()
{
        bool stat = false;
        createTopics();
        // Select MQTT server
        if (Ping.ping(mqtt_server))
        {
                mqttClient.setServer(mqtt_server, 1883);
                stat = true;
                alternativeMQTTserver = false;
                if (useSerial)
                {
                        Serial.println("MQTT SERVER: ");
                        Serial.println(mqtt_server);
                }
        }
        else if (Ping.ping(mqtt_server2))
        {
                mqttClient.setServer(mqtt_server2, 1883);
                if (useSerial)
                {
                        Serial.println("Connected to MQTT SERVER2: ");
                        Serial.println(mqtt_server2);
                }
                alternativeMQTTserver = true;
                stat = true;
        }
        else
        {
                mqttClient.setServer(MQTT_SERVER3, 1883);
                if (useSerial)
                {
                        Serial.println("Connected to EXTERNAL MQTT SERVER: ");
                        Serial.println(MQTT_SERVER3);
                }
                alternativeMQTTserver = true;
                stat = true;
        }
        // Set callback function
        if (stat)
        {
                mqttClient.setCallback(std::bind(&myIOT::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                subscribeMQTT();
        }
        else
        {
                if (useSerial)
                {
                        Serial.println("Not connected to MQTT server");
                }
        }
}
bool myIOT::subscribeMQTT()
{
        if (!mqttClient.connected())
        {
                long now = millis();
                if (now - lastReconnectAttempt > 5000)
                {
                        lastReconnectAttempt = now;

                        if (useSerial)
                        {
                                Serial.println("have wifi, entering MQTT connection");
                                Serial.print("Attempting MQTT connection...");
                        }
                        // Attempt to connect
                        char tempname[15];
                        sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());

                        if (mqttClient.connect(tempname, user, passw, _availTopic, 0, true, "offline"))
                        {
                                for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++)
                                {
                                        if (strcmp(topicArry[i], "") != 0)
                                        {
                                                mqttClient.subscribe(topicArry[i]);
                                        }
                                }
                                if (useSerial)
                                {
                                        Serial.println("connected");
                                }
                                if (firstRun)
                                {
                                        pub_log("<< PowerON Boot >>");
                                        if (!useResetKeeper)
                                        {
                                                firstRun = false;
                                                mqtt_detect_reset = 0;
                                                notifyOnline();
                                        }
                                        else
                                        { // using reset keeper
                                                mqttClient.publish(_availTopic, "resetKeeper", true);
                                        }
                                }
                                else
                                { // not first run
                                        notifyOnline();
                                }
                                return 1;
                        }
                        else
                        { // fail to connect MQTT
                                if (useSerial)
                                {
                                        Serial.print("failed, rc=");
                                        Serial.println(mqttClient.state());
                                }
                                return 0;
                        }
                }
        }
        else
        {
                return 1;
        }
}
void myIOT::createTopics()
{
        snprintf(_msgTopic, MaxTopicLength, "%s/Messages", prefixTopic);
        snprintf(_groupTopic, MaxTopicLength, "%s/All", prefixTopic);
        snprintf(_errorTopic, MaxTopicLength, "%s/log", prefixTopic);
        snprintf(_signalTopic, MaxTopicLength, "%s/Signal", prefixTopic);
        if (useTelegram)
        {
                snprintf(_telegramServer, MaxTopicLength, "%s/%s", prefixTopic, telegramServer);
        }

        if (strcmp(addGroupTopic, "") != 0)
        {
                char temptopic[MaxTopicLength];
                strcpy(temptopic, addGroupTopic);
                snprintf(addGroupTopic, MaxTopicLength, "%s/%s", prefixTopic, temptopic);

                snprintf(deviceTopic, MaxTopicLength, "%s/%s", addGroupTopic, _deviceName);
        }
        else
        {
                snprintf(deviceTopic, MaxTopicLength, "%s/%s", prefixTopic, _deviceName);
        }

        snprintf(_stateTopic, MaxTopicLength, "%s/State", deviceTopic);
        snprintf(_stateTopic2, MaxTopicLength, "%s/State_2", deviceTopic);
        snprintf(_availTopic, MaxTopicLength, "%s/Avail", deviceTopic);
}
void myIOT::callback(char *topic, byte *payload, unsigned int length)
{
        char incoming_msg[150];
        char msg[100];

        if (useSerial)
        {
                Serial.print("Message arrived [");
                Serial.print(topic);
                Serial.print("] ");
        }
        for (int i = 0; i < length; i++)
        {
                if (useSerial)
                {
                        Serial.print((char)payload[i]);
                }
                incoming_msg[i] = (char)payload[i];
        }
        incoming_msg[length] = 0;

        if (useSerial)
        {
                Serial.println("");
        }
        sprintf(mqqt_ext_buffer[0], "%s", topic);
        sprintf(mqqt_ext_buffer[1], "%s", incoming_msg);
        sprintf(mqqt_ext_buffer[2], "%s", deviceTopic);
        if (strcmp(topic, _availTopic) == 0 && useResetKeeper && firstRun)
        {
                firstRun_ResetKeeper(incoming_msg);
        }

        if (strcmp(incoming_msg, "boot") == 0)
        {
                sprintf(msg, "Boot:[%s]", bootTime);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ip") == 0)
        {
                char buf[16];
                sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
                sprintf(msg, "IP address:[%s]", buf);
                pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ota") == 0)
        {
                sprintf(msg, "OTA allowed for %d seconds", OTA_upload_interval / 1000);
                pub_msg(msg);
                allowOTA_clock = millis();
        }
        else if (strcmp(incoming_msg, "reset") == 0)
        {
                sendReset("MQTT");
        }
        else
        {
                if (extDefine)
                { // if this function was define then:
                        ext_mqtt(incoming_msg);
                }
        }
}
void myIOT::pub_msg(char *inmsg)
{
        char tmpmsg[250];
        get_timeStamp();

        if (mqttClient.connected())
        {
                sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic);
                msgSplitter(inmsg, 200, tmpmsg, "#");
        }
}
void myIOT::pub_state(char *inmsg, byte i)
{
        if (mqttClient.connected())
        {
                if (i == 0)
                {
                        mqttClient.publish(_stateTopic, inmsg, true);
                }
                else if (i == 1)
                {
                        mqttClient.publish(_stateTopic2, inmsg, true);
                }
        }
}
bool myIOT::pub_log(char *inmsg)
{
        char tmpmsg[150];
        get_timeStamp();
        sprintf(tmpmsg, "[%s] [%s] log: %s", timeStamp, deviceTopic, inmsg);

        if (mqttClient.connected())
        {
                mqttClient.publish(_errorTopic, tmpmsg);
                return 1;
        }
        else
        {
                return 0;
        }
}
void myIOT::msgSplitter(const char *msg_in, int max_msgSize, char *prefix, char *split_msg)
{
        char tmp[120];

        if (strlen(prefix) + strlen(msg_in) > max_msgSize)
        {
                int max_chunk = max_msgSize - strlen(prefix) - strlen(split_msg);
                int num = ceil((float)strlen(msg_in) / max_chunk);
                int pre_len;

                for (int k = 0; k < num; k++)
                {
                        sprintf(tmp, "%s %s%d: ", prefix, split_msg, k);
                        pre_len = strlen(tmp);
                        for (int i = 0; i < max_chunk; i++)
                        {
                                tmp[i + pre_len] = (char)msg_in[i + k * max_chunk];
                                tmp[i + 1 + pre_len] = '\0';
                        }
                        if (mqttClient.connected() == true)
                        {
                                mqttClient.publish(_msgTopic, tmp);
                        }
                }
        }
        else
        {
                if (mqttClient.connected() == true)
                {
                        sprintf(tmp, "%s %s", prefix, msg_in);
                        mqttClient.publish(_msgTopic, tmp);
                }
        }
}
int myIOT::inline_read(char *inputstr)
{
        char *pch;
        int i = 0;

        pch = strtok(inputstr, " ,.-");
        while (pch != NULL)
        {
                sprintf(inline_param[i], "%s", pch);
                pch = strtok(NULL, " ,.-");
                i++;
        }
        return i;
}
void myIOT::notifyOnline()
{
        mqttClient.publish(_availTopic, "online", true);
}
void myIOT::notifyOffline()
{
        mqttClient.publish(_availTopic, "offline", true);
}
void myIOT::pub_offline_errs()
{
        if (strcmp(bootErrors, "") != 0)
        {
                if (pub_log(bootErrors))
                {
                        strcpy(bootErrors, "");
                }
        }
}
void myIOT::firstRun_ResetKeeper(char *msg)
{
        if (strcmp(msg, "online") == 0)
        {
                mqtt_detect_reset = 1; // bad reboot
        }
        else
        {
                mqtt_detect_reset = 0; // ordinary boot
        }
        firstRun = false;
        notifyOnline();
}
void myIOT::register_err(char *inmsg)
{
        char temp[50];

        sprintf(temp, "--> %s", inmsg);
        strcat(bootErrors, temp);
}
void myIOT::send_tele_msg(char *msg)
{
        char t[150];
        sprintf(t, "[%s][%s]: %s", timeStamp, _deviceName, msg);
        mqttClient.publish(_telegramServer, t);
}
// ~~~~~~ Reset and maintability ~~~~~~
void myIOT::sendReset(char *header)
{
        char temp[150];

        sprintf(temp, "[%s] - Reset sent", header);

        if (useSerial)
        {
                Serial.println(temp);
        }
        if (strcmp(header, "null") != 0)
        {
                pub_msg(temp);
        }
        delay(1000);
        ESP.reset();
}
void myIOT::feedTheDog()
{
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries)
        {
                sendReset("Dog goes woof");
        }
}
void myIOT::acceptOTA()
{
        if (millis() - allowOTA_clock <= OTA_upload_interval)
        {
                ArduinoOTA.handle();
        }
}
void myIOT::startOTA()
{
        char OTAname[100];
        int m = 0;
        // create OTAname from deviceTopic
        for (int i = ((String)deviceTopic).lastIndexOf("/") + 1; i < strlen(deviceTopic); i++)
        {
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
                if (ArduinoOTA.getCommand() == U_FLASH)
                {
                        type = "sketch";
                }
                else
                { // U_SPIFFS
                        type = "filesystem";
                }

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                //    if (useSerial) {
                // Serial.println("Start updating " + type);
                //    }
                // Serial.end();
        });
        if (useSerial)
        { // for debug
                ArduinoOTA.onEnd([]() {
                        Serial.println("\nEnd");
                });
                ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
                });
                ArduinoOTA.onError([](ota_error_t error) {
                        Serial.printf("Error[%u]: ", error);
                        if (error == OTA_AUTH_ERROR)
                        {
                                Serial.println("Auth Failed");
                        }
                        else if (error == OTA_BEGIN_ERROR)
                        {
                                Serial.println("Begin Failed");
                        }
                        else if (error == OTA_CONNECT_ERROR)
                        {
                                Serial.println("Connect Failed");
                        }
                        else if (error == OTA_RECEIVE_ERROR)
                        {
                                Serial.println("Receive Failed");
                        }
                        else if (error == OTA_END_ERROR)
                        {
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
void myIOT::startWDT()
{
        wdt.attach(1, std::bind(&myIOT::feedTheDog, this)); // Start WatchDog
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ############################### FVars CLASS #################################
FVars::FVars(char *key, char *pref)
{
        sprintf(_key, "%s%s", pref, key);
}
bool FVars::getValue(int &ret_val)
{
        return json.getValue(_key, ret_val);
}
bool FVars::getValue(long &ret_val)
{
        return json.getValue(_key, ret_val);
}
bool FVars::getValue(char value[20])
{
        return json.getValue(_key, value);
}
void FVars::setValue(int val)
{
        json.setValue(_key, val);
}
void FVars::setValue(long val)
{
        json.setValue(_key, val);
}
void FVars::setValue(char *val)
{
        json.setValue(_key, val);
}

void FVars::remove()
{
        json.removeValue(_key);
}
void FVars::printFile()
{
        json.printFile();
}
void FVars::format()
{
        json.format();
}

// ~~~~~~~~~~~ TimeOut Class ~~~~~~~~~~~~
timeOUT::timeOUT(char *sw_num, int def_val)
    : endTimeOUT_inFlash(_key1, sw_num), inCodeTimeOUT_inFlash(_key2, sw_num),
      updatedTimeOUT_inFlash(_key3, sw_num), startTimeOUT_inFlash(_key4, sw_num),
      dailyTO_inFlash("TO.json", true)
{
        /* endTimeOUT_inFlash     -- Save clock when TO ends (sec from epoch)
           inCodeTimeOUT_inFlash  -- save value of TO defined in code [ minutes]
           updatedTimeOUT_inFlash -- value of TO defined using MQTT by user, overides inCode value [minutes]
         */

        int tempVal_inFlash = 0;
        inCodeTO = def_val; //[min]

        if (updatedTimeOUT_inFlash.getValue(updatedTO_inFlash) != true)
        { // not able to read
                updatedTimeOUT_inFlash.setValue(0);
        }

        if (endTimeOUT_inFlash.getValue(endTO_inFlash) != true)
        { // not able to read
                endTimeOUT_inFlash.setValue(0);
        }

        if (inCodeTimeOUT_inFlash.getValue(tempVal_inFlash) != true)
        {
                inCodeTimeOUT_inFlash.setValue(0);
        }
        else
        {
                if (tempVal_inFlash != inCodeTO)
                {
                        inCodeTimeOUT_inFlash.setValue(inCodeTO);
                }
        }
        if (startTimeOUT_inFlash.getValue(startTO_inFlash) != true)
        { // not able to read
                startTimeOUT_inFlash.setValue(0);
        }

        if (updatedTO_inFlash != 0)
        {
                _calc_TO = updatedTO_inFlash;
        }
        else
        {
                _calc_TO = inCodeTO;
        }
}
bool timeOUT::looper()
{
        dailyTO_looper(dailyTO);
        if (_calc_endTO > now())
        {
                return 1;
        }
        else
        {
                if (_inTO == true)
                {
                        switchOFF();
                }
                return 0;
        }
}
bool timeOUT::begin(bool newReboot)
{ // NewReboot come to not case of sporadic reboot

        // ~~~~~~~~ Check if stored end value stil valid ~~~~~~~~~~~~~~
        if (endTO_inFlash > now())
        {                                    // get saved value- still have to go
                _calc_endTO = endTO_inFlash; //clock time to stop
                switchON();
                return 1;
        }
        else if (endTO_inFlash > 0 && endTO_inFlash <= now())
        { // saved but time passed
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
        // else if (endTO_inFlash == 0 && newReboot == true) {                   // fresh start
        //         if (_calc_TO != 0) {
        //                 setNewTimeout(_calc_TO);
        //                 return 1;
        //         }
        //         else {
        //                 _calc_endTO = 0;
        //                 return 0;
        //         }
        // }
        else if (endTO_inFlash == 0)
        { // saved but time passed
                return 0;
        }
        sprintf(dTO_pubMsg, "");
}
int timeOUT::remain()
{
        if (_inTO == true)
        {
                return _calc_endTO - now(); //sec
        }
        else
        {
                return 0;
        }
}
void timeOUT::setNewTimeout(int to, bool mins)
{
        if (mins == true)
        {
                _calc_endTO = now() + to * 60; // when given in mintes -- which is default
        }
        else
        {
                _calc_endTO = now() + to;
        }
        endTimeOUT_inFlash.setValue(_calc_endTO); // store end_to to flash

        switchON();
}
void timeOUT::restart_to()
{
        setNewTimeout(_calc_TO);
}
void timeOUT::updateTOinflash(int TO)
{
        updatedTimeOUT_inFlash.setValue(TO);
        setNewTimeout(TO);
}
void timeOUT::restore_to()
{
        updatedTimeOUT_inFlash.setValue(0);
        endTimeOUT_inFlash.setValue(0);
        inCodeTimeOUT_inFlash.setValue(0);
}
void timeOUT::switchON()
{
        _inTO = true;
}
void timeOUT::switchOFF()
{
        endTimeOUT_inFlash.setValue(0);
        endNow();
        _inTO = false;
}
void timeOUT::endNow()
{
        _calc_endTO = now();
        if (dailyTO.onNow == true)
        { // for dailyTO
                dailyTO.onNow = false;
                sprintf(dTO_pubMsg, "DailyTimeOut: [End]");
        }
}
void timeOUT::convert_epoch2clock(long t1, long t2, char *time_str, char *days_str)
{
        byte days = 0;
        byte hours = 0;
        byte minutes = 0;
        byte seconds = 0;

        int sec2minutes = 60;
        int sec2hours = (sec2minutes * 60);
        int sec2days = (sec2hours * 24);
        int sec2years = (sec2days * 365);

        long time_delta = t1 - t2;

        days = (int)(time_delta / sec2days);
        hours = (int)((time_delta - days * sec2days) / sec2hours);
        minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

        sprintf(days_str, "%02d days", days);
        sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
long timeOUT::getStart_to()
{
        long getVal;
        startTimeOUT_inFlash.getValue(getVal);
        return getVal;
}
void timeOUT::updateStart(long clock)
{
        startTimeOUT_inFlash.setValue(clock);
}

int timeOUT::calc_dailyTO(dTO &dailyTO)
{
        int secs = dailyTO.off[2] - dailyTO.on[2];
        int mins = dailyTO.off[1] - dailyTO.on[1];
        int delt_h = dailyTO.off[0] - dailyTO.on[0];

        int total_time = secs + mins * 60 + delt_h * 60 * 60;
        if (total_time < 0)
        {
                total_time += 24 * 60 * 60;
        }
        return total_time;
}
void timeOUT::dailyTO_looper(dTO &dailyTO)
{
        time_t t = now();
        if (dailyTO.flag == true)
        {
                if (dailyTO.onNow == false)
                { // start
                        if (hour(t) == dailyTO.on[0] && minute(t) == dailyTO.on[1] && second(t) == dailyTO.on[2])
                        {
                                int tot_time = calc_dailyTO(dailyTO);
                                char time_str[20], date_str[20];
                                convert_epoch2clock(now() + tot_time, now(), time_str, date_str);
                                setNewTimeout(tot_time, false);
                                dailyTO.onNow = true;
                                sprintf(dTO_pubMsg, "DailyTimeOut: [Start] until [%02d:%02d:%02d]", dailyTO.off[0], dailyTO.off[1], dailyTO.off[2]);
                        }
                }
        }
}
void timeOUT::check_dailyTO_inFlash(dTO &dailyTO, int x)
{
        char temp[10];
        int retVal;

        if (dailyTO_inFlash.file_exists())
        {
                sprintf(temp, "%s_%d", clock_fields[3], x);
                if (dailyTO_inFlash.getValue(temp, retVal))
                {

                        if (retVal)
                        { //only if flag is to read values from flash
                                for (int m = 0; m < sizeof(clock_fields) / sizeof(clock_fields[0]); m++)
                                {
                                        sprintf(temp, "%s_%d", clock_fields[m], x);
                                        if (m == 0 || m == 1)
                                        { // clock fileds only -- on or off
                                                for (int i = 0; i < items_each_array[m]; i++)
                                                {
                                                        dailyTO_inFlash.getArrayVal(temp, i, retVal);
                                                        if (retVal >= 0)
                                                        {
                                                                if ((i == 0 && retVal <= 23) || (i > 0 && retVal <= 59))
                                                                { //valid time
                                                                        if (m == 0)
                                                                        {
                                                                                dailyTO.on[i] = retVal;
                                                                        }
                                                                        else
                                                                        {
                                                                                dailyTO.off[i] = retVal;
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                        else
                                        { // for flag value
                                                dailyTO_inFlash.getValue(temp, retVal);
                                                if (retVal == 0 || retVal == 1)
                                                { //flag on or off
                                                        if (m == 2)
                                                        {
                                                                dailyTO.flag = retVal;
                                                        }
                                                        else if (m == 3)
                                                        {
                                                                dailyTO.useFlash = retVal;
                                                        }
                                                }
                                                else
                                                {
                                                        dailyTO_inFlash.setValue(temp, 0);
                                                }
                                        }
                                }
                        }
                }
                else
                { // create NULL values
                        store_dailyTO_inFlash(defaultVals, x);
                }
        }
}
void timeOUT::store_dailyTO_inFlash(dTO &dailyTO, int x)
{
        char temp[10];
        for (int m = 0; m < sizeof(clock_fields) / sizeof(clock_fields[0]); m++)
        {
                sprintf(temp, "%s_%d", clock_fields[m], x);
                if (m == 0)
                {
                        for (int i = 0; i < items_each_array[m]; i++)
                        {
                                dailyTO_inFlash.setArrayVal(temp, i, dailyTO.on[i]);
                        }
                }
                else if (m == 1)
                {
                        for (int i = 0; i < items_each_array[m]; i++)
                        {
                                dailyTO_inFlash.setArrayVal(temp, i, dailyTO.off[i]);
                        }
                }
                else if (m == 2)
                {
                        dailyTO_inFlash.setValue(temp, dailyTO.flag);
                }
                else if (m == 3)
                {
                        dailyTO_inFlash.setValue(temp, dailyTO.useFlash);
                }
        }
}
void timeOUT::restart_dailyTO(dTO &dailyTO)
{
        time_t t = now();
        dTO temp_dTO = {{hour(t), minute(t), second(t)}, {dailyTO.off[0], dailyTO.off[1], dailyTO.off[2]}, 1, 1, 0};
        int tot_time = calc_dailyTO(temp_dTO);

        setNewTimeout(tot_time, false);
        dailyTO.onNow = true;
}

// ~~~~~~~~~~~ myTelegram Class ~~~~~~~~~~~~
myTelegram::myTelegram(char *Bot, char *chatID, int checkServer_interval, char *ssid, char *password) : bot(Bot, client)
{
        sprintf(_bot, Bot);
        sprintf(_chatID, chatID);
        sprintf(_ssid, ssid);
        sprintf(_password, password);
}
void myTelegram::handleNewMessages(int numNewMessages)
{
        char sendmsg[500];

        for (int i = 0; i < numNewMessages; i++)
        {
                String chat_id = String(bot.messages[i].chat_id);
                String text = bot.messages[i].text;
                String from_name = bot.messages[i].from_name;
                if (from_name == "")
                        from_name = "Guest";
                _ext_func(text, from_name, chat_id, sendmsg);

                if (strcmp(sendmsg, "") != 0)
                {
                        bot.sendMessage(chat_id, sendmsg, "");
                }
        }
}
void myTelegram::begin(cb_func2 funct)
{

        _ext_func = funct; // call to external function outside of clss

        if (WiFi.status() != WL_CONNECTED)
        {
                WiFi.mode(WIFI_STA);
                WiFi.disconnect();
                delay(100);

                // Serial.print("Connecting Wifi: ");
                // Serial.println(_ssid);
                // WiFi.begin(_ssid, _password);

                while (WiFi.status() != WL_CONNECTED)
                {
                        // Serial.print(".");
                        delay(500);
                }
                WiFi.setAutoReconnect(true);
        }

        // Serial.println("");
        // Serial.println("WiFi connected");
        // Serial.print("IP address: ");
        // Serial.println(WiFi.localIP());

        client.setInsecure();
}
void myTelegram::send_msg(char *msg)
{
        bot.sendMessage(_chatID, msg, "");
}
void myTelegram::looper()
{

        if (millis() > _Bot_lasttime + _Bot_mtbs)
        {
                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

                while (numNewMessages)
                {
                        handleNewMessages(numNewMessages);
                        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                }
                _Bot_lasttime = millis();
        }
}

// ~~~~~~~~~~~~~~~~ mySwitch ~~~~~~~~~~~~~~
mySwitch::mySwitch(int switchPin, int timeout_val, char *name)
    : TOswitch(name, timeout_val)
{
        _switchPin = switchPin;
        sprintf(_switchName, "%s", name);
}
void mySwitch::begin()
{
        if (useSerial)
        {
                Serial.begin(9600);
        }

        if (useInput && inputPin != -1)
        {
                pinMode(inputPin, INPUT_PULLUP);
        }

        pinMode(_switchPin, INPUT);

        _current_state = 0;
        if (usePWM)
        {
                analogWrite(_switchPin, _current_state);
        }
        else
        {
                digitalWrite(_switchPin, _current_state);
        }

        TOswitch.begin(false);
        if (TOswitch.remain() > 0)
        {
                badBoot = true;
        }
        if (useDailyTO)
        {
                _start_dailyTO();
        }
}
void mySwitch::changePower(float val)
{
        if (val != _current_state)
        {
                if (val >= max_power)
                {
                        val = max_power;
                }
                else if (val == 0)
                {
                        val = 0;
                }
                else if (val <= min_power)
                {
                        val = min_power;
                }
                _current_state = val;
                analogWrite(_switchPin, val * PWM_RES);
        }
}
void mySwitch::switchIt(char *txt1, float state)
{
        char msg1[20];
        char msg2[20];
        if (state != _current_state)
        {
                if (usePWM)
                {
                        if (state == 0.0)
                        {
                                // turning off
                                changePower(state);
                                sprintf(_switchMSG, "%s: Switched [Off]", txt1);
                        }
                        else if (_current_state == 0.0 && state > 0)
                        {
                                // turning on
                                changePower(state);
                                sprintf(_switchMSG, "%s: Switched [On] [%.1f%%] power", txt1, _current_state * 100);
                                if (usetimeOUT)
                                {
                                        TOswitch.restart_to();
                                }
                        }
                        else
                        {
                                changePower(state);
                                sprintf(_switchMSG, "%s: Switched to [%.1f%%] power", txt1, _current_state * 100);
                        }
                }
                else
                {
                        if (state != _current_state)
                        {
                                digitalWrite(_switchPin, state);
                                _current_state = state;
                                sprintf(_switchMSG, "%s: Switched [%s]", txt1, (int)_current_state ? "On" : "Off");
                                if (usetimeOUT && _current_state == 1)
                                {
                                        TOswitch.restart_to();
                                }
                        }
                }
                TOswitch.convert_epoch2clock(now() + TOswitch.remain(), now(), msg1, msg2);

                if (usetimeOUT && TOswitch.remain() > 0)
                {
                        if (_current_state == 0.0)
                        {
                                TOswitch.endNow();
                        }
                        else
                        {
                                sprintf(msg2, " timeLeft[%s]", msg1);
                                strcat(_switchMSG, msg2);
                        }
                }
        }
}
void mySwitch::_checkSwitch_Pressed(int swPin, bool momentary)
{
        momentary = is_momentery;
        if (momentary)
        {
                if (digitalRead(swPin) == SwitchOn)
                {
                        delay(50);
                        if (digitalRead(swPin) == SwitchOn)
                        {
                                if (usePWM) // for mosfet switching
                                {
                                        if (_current_state + step_power > max_power)
                                        {
                                                switchIt("Button", 0);
                                        }
                                        else
                                        {
                                                switchIt("Button", _current_state + step_power);
                                        }
                                }
                                else // for relay switching
                                {
                                        if (_current_state == 1)
                                        {
                                                switchIt("Button1", 0);
                                        }
                                        else
                                        {
                                                switchIt("Button1", 1);
                                        }
                                }
                        }
                }
        }
        else
        {
                static bool inputState;
                if (digitalRead(swPin) != inputState)
                {
                        delay(50);
                        if (digitalRead(swPin) != inputState)
                        {
                                inputState = digitalRead(swPin);
                                if (inputState == SwitchOn && digitalRead(_switchPin) != RelayOn)
                                { // turn in TO
                                        TOswitch.restart_to();
                                }
                                else if (inputState != SwitchOn)
                                { // turn off
                                        if (TOswitch.remain() > 0)
                                        { // turn off when in TO
                                                TOswitch.endNow();
                                        }
                                        else if (digitalRead(_switchPin) == RelayOn && TOswitch.remain() == 0)
                                        { // turn off when only ON
                                                switchIt("Button:", 0);
                                        }
                                }
                        }
                }
        }
        delay(100);
}
void mySwitch::_TOlooper(int det_reset)
{
        if (det_reset != 2)
        {
                static bool last_relayState = false;
                bool relayState = TOswitch.looper();

                if (relayState != last_relayState)
                { // change state (ON <-->OFF)
                        if (relayState == 0 && _current_state > 0)
                        {
                                switchIt("TimeOut", relayState);
                        }
                        else if (_current_state == 0 && relayState == 1)
                        {
                                if (usePWM)
                                {
                                        switchIt("TimeOut", def_power);
                                }
                                else
                                {
                                        switchIt("TimeOut", relayState);
                                }
                        }
                }
                // if (usePWM)
                // {
                //         if (relayState == 0 && _current_state > 0)
                //         {
                //                 switchIt("TimeOut", 0);
                //         }
                //         else if (_current_state == 0 && relayState == 1)
                //         {

                //                 switchIt("TimeOut", def_power);
                //         }
                // }
                // else
                // {
                //         if (relayState == 0 && _current_state == 1)
                //         {
                //                 switchIt("TimeOut", relayState);
                //         }
                //         else if (_current_state == 0 && relayState == 1)
                // }
                last_relayState = relayState;
        }
}
void mySwitch::_start_dailyTO()
{
        memcpy(TOswitch.dailyTO.on, START_dailyTO, sizeof(START_dailyTO));
        memcpy(TOswitch.dailyTO.off, END_dailyTO, sizeof(END_dailyTO));
        TOswitch.dailyTO.flag = useDailyTO;
        TOswitch.check_dailyTO_inFlash(TOswitch.dailyTO, 0);
}
bool mySwitch::postMessages(char outmsg[150])
{
        if (strcmp(TOswitch.dTO_pubMsg, "") != 0 && useDailyTO)
        {
                sprintf(outmsg, "%s", TOswitch.dTO_pubMsg);
                sprintf(TOswitch.dTO_pubMsg, "%s", "");
                return 1;
        }
        if (strcmp(_switchMSG, "") != 0)
        {
                sprintf(outmsg, "%s", _switchMSG);
                sprintf(_switchMSG, "%s", "");
                return 1;
        }

        return 0;
}
void mySwitch::adHOC_timeout(int mins, bool inMinutes)
{
        TOswitch.setNewTimeout(mins, inMinutes);
}
void mySwitch::looper(int det_reset)
{
        TOswitch.looper();
        _TOlooper(det_reset);
        if (useInput)
        {
                _checkSwitch_Pressed(inputPin);
        }
        if (useEXTtrigger)
        {
                _extTrig_looper();
        }
}
void mySwitch::extTrig_cb(bool det, bool retrig, char *trig_name)
{
        _ext_det = det;
        _retrig = retrig;
        _trig_name = trig_name;
}
void mySwitch::_extTrig_looper()
{
        static bool trig_lastState = false;

        if (ext_trig_signal != trig_lastState)
        {
                Serial.println(ext_trig_signal);
                if (ext_trig_signal == _ext_det) //trig is ON according defined by det
                {
                        if (TOswitch.remain() > 0)
                        {
                                if (_retrig)
                                {
                                        TOswitch.restart_to(); // restart timeout
                                }
                        }
                        else
                        {
                                switchIt(_trig_name, def_power); // set ON
                        }
                }
        }
        trig_lastState = ext_trig_signal;
}
void mySwitch::setdailyTO(const int start_clk[], const int end_clk[])
{
        for (int i = 0; i < 3; i++)
        {
                START_dailyTO[i] = start_clk[i];
                END_dailyTO[i] = end_clk[i];
        }
}