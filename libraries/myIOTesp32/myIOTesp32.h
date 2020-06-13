#ifndef myIOT32_h
#define myIOT32_h
#include "Arduino.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "time.h"
#include "WiFi.h"
#include "secrets.h"
#include <ESP32Ping.h>
#include <PubSubClient.h>

#include <Ticker.h>
#include <ArduinoJson.h>

class myIOT32
{
#define VER "iot32_ver_0.4"
#define JDOC_SIZE 300
#define RECON_WIFI 60       // sec to reconnect
#define RECON_MQTT 30       // sec to reconnect
#define NO_NETWORK_RESET 10 // minutes
#define UPDATE_STATUS_MINS 1 // minutes

private:
    // MQTT topics
    static const int MaxTopicLength = 64; //topics
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

    char _incmoing_wakeMSG[100];
    // Wifi & MQTT config
    int _mqtt_port;
    char *_wifi_ssid;
    char *_wifi_pass;
    char *_mqtt_server;
    bool _alternativeMQTTserver = false;
    char *_user = "";
    char *_passw = "";
    char *_devTopic = "DEVICE_TOPIC OF MQTT";
    long _networkerr_clock = 0;

    // Clock
    struct tm _timeinfo;
    time_t _epoch_time;

    volatile int _wdtResetCounter = 0;
    const int _wdtMaxRetries = 30; //seconds to bITE

public:
    bool useSerial = false;
    bool useOTA = true;
    bool useWDT = true;
    bool useResetKeeper = true;
    bool networkOK = false;
    long unsigned allowOTA_clock = 0;
    int bootType = 2; // 2 - init; 1 - resetboot; 0- regular boot
    char prefixTopic[MaxTopicLength];
    char deviceTopic[MaxTopicLength];
    char addGroupTopic[MaxTopicLength];
    char telegramServer[MaxTopicLength];
    char inline_param[6][20]; //values from user

    struct status
    {
        long last_keepalive;
        long boot_clock;
        long nextWake;
        int sleeptime;

        char ip[20];
        char *wake_cmd;
        char *devicetopic;

        bool input1;
        bool input2;
        bool output1;
        bool output2;
        bool wake_status;
    };
    status DeviceStatus = {0, 0, 0, 0, "", "NO_WAKE_CMD", deviceTopic, false, false, false, false, false};

    WiFiClient espClient;
    PubSubClient mqttClient;
    Ticker wdt;

public:
    myIOT32(char *devTopic = "no-name", char *ssid = SSID_ID, char *wifi_p = PASS_WIFI,
            char *mqtt_broker = MQTT_SERVER1, char *mqttU = MQTT_USER, char *mqttP = MQTT_PASS,
            int port = 1883);
    void looper();
    void start();
    bool startMQTT();
    void pub_msg(char *msg);
    void pub_Status(char *statusmsg);
    void pub_nextWake(char *inmsg);
    void pub_log(char *inmsg);
    void getTimeStamp(char ret_timeStamp[25]);
    void sendReset(char *header);
    void createWakeJSON();
    bool checkInternet(char *externalSite, byte pings = 1);

private:
    bool MQTTloop();
    void startNTP(const int gmtOffset_sec, const int daylightOffset_sec, const char *ntpServer);
    bool startWifi();
    void getTime();
    void MQTTcallback(char *topic, byte *payload, unsigned int length);
    void createTopics();
    bool connectMQTT();
    void subscribeMQTT();
    bool _selectMQTTserver();
    void _notifyOnline();
    void _startOTA();
    void _OTAlooper();
    void _feedTheDog();
    void _startWDT();
    void _createStatusJSON();
    void _getMQTT2JSON(char *input_str);
    void _updateKeepAlive();
    void _networkflags(bool s);
    int _inline_read(char *inputstr);
};

#endif