#ifndef myIOT32_h
#define myIOT32_h
#include "Arduino.h"

// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>
#include "time.h"
#include "WiFi.h"
#include "secrets.h"
#include <PubSubClient.h>

class myIOT32
{

    static const int MaxTopicLength = 64; //topics

private:
    // MQTT topics
    char _msgTopic[MaxTopicLength];
    char _groupTopic[MaxTopicLength];
    char _errorTopic[MaxTopicLength];
    char _deviceName[MaxTopicLength];
    char _availTopic[MaxTopicLength];
    char _stateTopic[MaxTopicLength];
    char _statusTopic[MaxTopicLength];
    char _wakeTopic[MaxTopicLength];
    char _telegramServer[MaxTopicLength];
    char *topicArry[6] = {deviceTopic, _groupTopic, _availTopic, addGroupTopic, _wakeTopic, _statusTopic};

    struct tm _timeinfo;
    time_t _epoch_time;
    int _mqtt_port;
    char *_wifi_ssid; // = "WIFI_NETWORK_BY_USER";
    char *_wifi_pass; // = "WIFI__passwORD_BY_USER";
    char *_mqtt_server;
    char *_user = "";
    char *_passw = "";
    char *_devTopic = "DEVICE_TOPIC OF MQTT";
    long _networkerr_clock = 0;

public:
    char prefixTopic[MaxTopicLength];
    char deviceTopic[MaxTopicLength];
    char addGroupTopic[MaxTopicLength];
    char telegramServer[MaxTopicLength];

    WiFiClient espClient;
    PubSubClient mqttClient;

public:
    myIOT32(char *devTopic = "no-name", char *ssid = SSID_ID, char *wifi_p = PASS_WIFI,
            char *mqtt_broker = MQTT_SERVER1, char *mqttU = MQTT_USER, char *mqttP = MQTT_PASS, int port = 1883);
    void looper();
    void start();
    void mqtt_pubmsg(char *msg);
    bool use_wifi = true;
    bool startMQTT();

private:
    bool MQTTloop();
    void startNTP(const int gmtOffset_sec, const int daylightOffset_sec, const char *ntpServer);
    bool startWifi();
    void getTime();
    void MQTTcallback(char *topic, byte *payload, unsigned int length);
    void createTopics();
    bool connectMQTT();
    void subscribeMQTT();
    void _notifyOnline();
};

#endif