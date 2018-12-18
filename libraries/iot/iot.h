#ifndef iot_h
#define iot_h

#define deviceTopic "HomePi/Dvir/Windows/Saloon1"

#include "Arduino.h"
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

class iot
{
  public:
    iot( int ssid,  int pwd);
  private:
    // Service flags
    bool useNetwork = true;
    bool useWDT = true;
    bool useSerial = false;
    bool useOTA = true;
    bool runPbit = false;
    int networkID = 1;  // 0: HomeNetwork,  1:Xiaomi_D6C8


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
    const char* topicArry[2] = {deviceTopic, groupTopic};
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

    void startNetwork();
    void startMQTT();
    void createTopics(const char *devTopic, char *state, char *avail);
    void startNTP();
    int subscribeMQTT();
    void pub_msg(char *inmsg);
    void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg);
    void get_timeStamp();
    void sendReset(char *header);
    void feedTheDog();
    void acceptOTA();

    int _pin;
    int _SSID;
    int _PWD;


};

#endif
