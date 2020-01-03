#include "Arduino.h"
#include "myIOT_2.h"


// ~~~~~~ myIOT CLASS ~~~~~~~~~~~ //
myIOT_2::myIOT_2(char *devTopic, char *key)
    : mqttClient(espClient)
{
        strcpy(_deviceName, devTopic); // for OTA only
}
void myIOT_2::start_services(cb_func funct, char *ssid, char *password, char *mqtt_user, char *mqtt_passw, char *mqtt_broker)
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
void myIOT_2::looper()
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
bool myIOT_2::startWifi(char *ssid, char *password)
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
void myIOT_2::start_network_services()
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
void myIOT_2::network_looper()
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
void myIOT_2::start_clock()
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
bool myIOT_2::startNTP()
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
                return 1;
        }
        if (year(t) == 1970)
        {
                register_err("NTP Fail obtain valid Clock");
                return 0;
        }
}
void myIOT_2::get_timeStamp(time_t t)
{
        if (t == 0)
        {
                t = now();
        }
        sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
void myIOT_2::return_clock(char ret_tuple[20])
{
        time_t t = now();
        sprintf(ret_tuple, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}
void myIOT_2::return_date(char ret_tuple[20])
{
        time_t t = now();
        sprintf(ret_tuple, "%02d-%02d-%02d", year(t), month(t), day(t));
}
bool myIOT_2::checkInternet(char externalSite[40])
{
        return Ping.ping(externalSite);
}

// ~~~~~~~ MQTT functions ~~~~~~~
void myIOT_2::startMQTT()
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
                mqttClient.setCallback(std::bind(&myIOT_2::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
bool myIOT_2::subscribeMQTT()
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
void myIOT_2::createTopics()
{
        snprintf(_msgTopic, MaxTopicLength, "%s/Messages", prefixTopic);
        snprintf(_groupTopic, MaxTopicLength, "%s/All", prefixTopic);
        snprintf(_errorTopic, MaxTopicLength, "%s/log", prefixTopic);
        snprintf(_signalTopic, MaxTopicLength, "%s/Signal", prefixTopic);

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
void myIOT_2::callback(char *topic, byte *payload, unsigned int length)
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
void myIOT_2::pub_msg(char *inmsg)
{
        char tmpmsg[250];
        get_timeStamp();

        if (mqttClient.connected())
        {
                sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic);
                msgSplitter(inmsg, 200, tmpmsg, "#");
        }
}
void myIOT_2::pub_state(char *inmsg, byte i)
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
bool myIOT_2::pub_log(char *inmsg)
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
void myIOT_2::msgSplitter(const char *msg_in, int max_msgSize, char *prefix, char *split_msg)
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
int myIOT_2::inline_read(char *inputstr)
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
void myIOT_2::notifyOnline()
{
        mqttClient.publish(_availTopic, "online", true);
}
void myIOT_2::notifyOffline()
{
        mqttClient.publish(_availTopic, "offline", true);
}
void myIOT_2::pub_offline_errs()
{
        if (strcmp(bootErrors, "") != 0)
        {
                if (pub_log(bootErrors))
                {
                        strcpy(bootErrors, "");
                }
        }
}
void myIOT_2::firstRun_ResetKeeper(char *msg)
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
void myIOT_2::register_err(char *inmsg)
{
        char temp[50];

        sprintf(temp, "--> %s", inmsg);
        strcat(bootErrors, temp);
}
// ~~~~~~ Reset and maintability ~~~~~~
void myIOT_2::sendReset(char *header)
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
void myIOT_2::feedTheDog()
{
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries)
        {
                sendReset("Dog goes woof");
        }
}
// void myIOT::acceptOTA()
// {
//         if (millis() - allowOTA_clock <= OTA_upload_interval)
//         {
//                 ArduinoOTA.handle();
//         }
// }
// void myIOT::startOTA()
// {
//         char OTAname[100];
//         int m = 0;
//         // create OTAname from deviceTopic
//         for (int i = ((String)deviceTopic).lastIndexOf("/") + 1; i < strlen(deviceTopic); i++)
//         {
//                 OTAname[m] = deviceTopic[i];
//                 OTAname[m + 1] = '\0';
//                 m++;
//         }

//         allowOTA_clock = millis();

//         // Port defaults to 8266
//         ArduinoOTA.setPort(8266);

//         // Hostname defaults to esp8266-[ChipID]
//         ArduinoOTA.setHostname(OTAname);

//         // No authentication by default
//         // ArduinoOTA.setPassword("admin");

//         // Password can be set with it's md5 value as well
//         // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
//         // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

//         ArduinoOTA.onStart([]() {
//                 String type;
//                 if (ArduinoOTA.getCommand() == U_FLASH)
//                 {
//                         type = "sketch";
//                 }
//                 else
//                 { // U_SPIFFS
//                         type = "filesystem";
//                 }

//                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//                 //    if (useSerial) {
//                 // Serial.println("Start updating " + type);
//                 //    }
//                 // Serial.end();
//         });
//         if (useSerial)
//         { // for debug
//                 ArduinoOTA.onEnd([]() {
//                         Serial.println("\nEnd");
//                 });
//                 ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//                         Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//                 });
//                 ArduinoOTA.onError([](ota_error_t error) {
//                         Serial.printf("Error[%u]: ", error);
//                         if (error == OTA_AUTH_ERROR)
//                         {
//                                 Serial.println("Auth Failed");
//                         }
//                         else if (error == OTA_BEGIN_ERROR)
//                         {
//                                 Serial.println("Begin Failed");
//                         }
//                         else if (error == OTA_CONNECT_ERROR)
//                         {
//                                 Serial.println("Connect Failed");
//                         }
//                         else if (error == OTA_RECEIVE_ERROR)
//                         {
//                                 Serial.println("Receive Failed");
//                         }
//                         else if (error == OTA_END_ERROR)
//                         {
//                                 Serial.println("End Failed");
//                         }
//                 });
//                 // ArduinoOTA.begin();
//                 // Serial.println("Ready");
//                 // Serial.print("IP address: ");
//                 // Serial.println(WiFi.localIP());
//         }

//         ArduinoOTA.begin();
// }
void myIOT_2::startWDT()
{
        wdt.attach(1, std::bind(&myIOT::feedTheDog, this)); // Start WatchDog
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
