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
    char prefixTopic[MaxTopicLength];
    char deviceTopic[MaxTopicLength];
    char addGroupTopic[MaxTopicLength];
    char telegramServer[MaxTopicLength];
    char MQTTmsgtopic[50];
    char MQTTlogtopic[50];
    char MQTTavltopic[50];
    char MQTTdevtopic[50];
    char MQTTlastctopic[50];
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
    WiFiClient espClient;
    PubSubClient mqttClient;
    // WiFiClient espClient;
    // PubSubClient mqttClient;

public:
    myIOT32(char *devTopic = "no-name", char *ssid = SSID_ID, char *wifi_p = PASS_WIFI, char *mqtt_broker = MQTT_SERVER1, char *mqttU = MQTT_USER, char *mqttP = MQTT_PASS, int port = 1883);
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
};

#endif