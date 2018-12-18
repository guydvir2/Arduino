
#include "Arduino.h"
#include "iot.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker wdt;


iot::iot( int ssid,  int pwd){
  Serial.begin(9600);
  _SSID = ssid;
  _PWD = pwd;
  startNetwork();
  startMQTT();
  startNTP();
}

void iot::startNetwork() {
        long startWifiConnection = millis();


        WiFi.mode(WIFI_STA);
        WiFi.begin("123", "123");
        WiFi.setAutoReconnect(true);

        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED){
                delay(500);
        }


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

        if (useSerial) {
                Serial.println("Sending Reset command");
        }
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
