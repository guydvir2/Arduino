
#include "Arduino.h"
#include "iot.h"

WiFiClient espClient;
// PubSubClient mqttClient(espClient);
// Ticker wdt;


iot::iot(){
        Serial.begin(9600);
        Serial.println("<< Start >>");

        startNetwork();
        // startOTA();
        // wdt.attach(1, feedTheDog);

}

void iot::startNetwork() {
        long startWifiConnection = millis();

        // selectNetwork();
        WiFi.mode(WIFI_STA);
        WiFi.begin("Xiaomi_D6C8", "guyd5161");
        // WiFi.setAutoReconnect(true);

        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED){ //} && millis() - startWifiConnection < WIFItimeOut) {
                delay(1000);
                Serial.println("WAIT");
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("OK");

                // startMQTT();
                // startNTP();
                // get_timeStamp();
                // strcpy(bootTime, timeStamp);
                // subscribeMQTT();                                                                                                                                                                                                                                                                                                                                                                       b6
        }


}
void iot::selectNetwork() {
        if (networkID == 1 ) {
                ssid = ssid_1;
                mqtt_server = mqtt_server_1;
        }
        else {
                ssid = ssid_0;
                mqtt_server = mqtt_server_0;
        }
}
int iot::networkStatus(){
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
void iot::network_check(){
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
void iot::startOTA() {
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
void iot::startMQTT() {
        createTopics(deviceTopic, stateTopic, availTopic);
        mqttClient.setServer("192.168.2.200", 1883);
        // mqttClient.setCallback(callback);
}
void iot::startNTP() {
        NTP.begin("pool.ntp.org", 2, true);
        NTP.setInterval(1000 * 3600 * clockUpdateInt);
}
void iot::createTopics(const char *devTopic, char *state, char *avail) {
        sprintf(state, "%s/State", devTopic);
        sprintf(avail, "%s/Avail", devTopic);
}
int iot::subscribeMQTT() {
        long startClock = millis();

        // verify wifi connected
        if (WiFi.status() == WL_CONNECTED) {
                // if (useSerial) {
                //         Serial.println("have wifi, entering MQTT connection");
                // }
                while (!mqttClient.connected() && mqttFailCounter <= MQTTretries) {
                        // if (useSerial) {
                        //         Serial.print("Attempting MQTT connection...");
                        // }

                        // Attempt to connect
                        if (mqttClient.connect(deviceName, user, passw, availTopic, 0, true, "offline")) {
                                // if (useSerial) {
                                //         Serial.println("connected");
                                // }
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
                                // if (useSerial) {
                                //         Serial.print("failed, rc=");
                                //         Serial.print(mqttClient.state());
                                //         Serial.print("number of fails to reconnect MQTT :");
                                //         Serial.println(mqttFailCounter);
                                // }
                                mqttFailCounter++;
                        }
                }

                // Failed to connect MQTT adter retries
                // if (useSerial) {
                //         Serial.println("Exit without connecting MQTT");
                // }
                mqttFailCounter = 0;
                return 0;
        }
        else {
                // if (useSerial) {
                //         Serial.println("Not connected to Wifi, abort try to connect MQTT broker");
                // }
                return 0;
        }
}
void iot::pub_msg(char *inmsg) {
        char tmpmsg[150];

        if (useNetwork == true && mqttConnected == true) {
                get_timeStamp();
                sprintf(tmpmsg, "[%s] [%s]", timeStamp, deviceTopic );
                msgSplitter(inmsg, 95, tmpmsg, "#" );
        }
}
void iot::msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg) {
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
void iot::get_timeStamp() {
        time_t t = now();
        sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
void iot::sendReset(char *header) {
        char temp[150];

        // if (useSerial) {
        //         Serial.println("Sending Reset command");
        // }
        if (strcmp(header, "null")!=0) {
                sprintf(temp, "[%s] - Reset sent", header);
                pub_msg(temp);
        }
        ESP.reset();
}
void iot::feedTheDog() {
        wdtResetCounter++;
        if (wdtResetCounter >= wdtMaxRetries) {
                sendReset("WatchDog woof");
        }
}
void iot::acceptOTA() {
        if (millis() - OTAcounter <= OTA_upload_interval) {
                ArduinoOTA.handle();
        }
}
