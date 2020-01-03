#ifndef myIOT_2_h
#define myIOT_2_h

#define jfile "myfile.json"

#include "secrets_2.h"
#include "Arduino.h"

#include "WiFi.h"

#include <PubSubClient.h> //MQTT
#include <Ticker.h>       //WDT
#include <NtpClientLib.h>

// // OTA libraries
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>

// // Telegram libraries
// #include <UniversalTelegramBot.h>
// #include <WiFiClientSecure.h>

// define generic functiobs
typedef void (*cb_func)(char msg1[50]);
typedef void (*cb_func2)(String msg1, String msg2, String msg3, char *msg4);

class myIOT_2
{
  static const int MaxTopicLength = 64; //topics

public:
  WiFiClient espClient;
  PubSubClient mqttClient;
  Ticker wdt;

  myIOT_2(char *devTopic, char *key = "failNTPcount");
  void start_services(cb_func funct, char *ssid = SSID_ID, char *password = PASS_WIFI, char *mqtt_user = MQTT_USER, char *mqtt_passw = MQTT_PASS, char *mqtt_broker = MQTT_SERVER1);
  void looper();
  // void startOTA();
  void get_timeStamp(time_t t = 0);
  void return_clock(char ret_tuple[20]);
  void return_date(char ret_tuple[20]);
  bool checkInternet(char externalSite[40]);

  void sendReset(char *header);
  void notifyOnline();
  void notifyOffline();
  void pub_state(char *inmsg, byte i = 0);
  void pub_msg(char *inmsg);
  bool pub_log(char *inmsg);
  int inline_read(char *inputstr);

  // ~~~~~~ Services ~~~~~~~~~
  bool useSerial = false;
  bool useWDT = true;
  bool useOTA = true;
  bool extDefine = false; // must to set to true in order to use EXtMQTT
  bool useResetKeeper = false;
  bool resetFailNTP = false;
  bool useTelegram = false;
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  char inline_param[6][20]; //values from user

  bool alternativeMQTTserver = false;
  bool noNetwork_flag = false;
  byte mqtt_detect_reset = 2;
  char prefixTopic[MaxTopicLength];
  char deviceTopic[MaxTopicLength];
  char addGroupTopic[MaxTopicLength];

  const char *ver = "iot_6.6";
  char timeStamp[20];

private:
  char *Ssid;
  char *Password;
  cb_func ext_mqtt;
  cb_func2 ext_telegram;

  // time interval parameters

  const int clockUpdateInt = 60 * 60 * 5;              // seconds to update NTP
  const int WIFItimeOut = (1000 * 60) * 1 / 2;         // 30 sec try to connect WiFi
  const int OTA_upload_interval = (1000 * 60) * 10;    // 10 minute to try OTA
  const long time2Reset_noNetwork = (1000 * 60) * 30L; // minutues pass without any network
  volatile int wdtResetCounter = 0;
  const int wdtMaxRetries = 30; //seconds to bITE
  long noNetwork_Clock = 0;     // clock
  long allowOTA_clock = 0;      // clock
  long lastReconnectAttempt = 0;
  // ############################

  //MQTT broker parameters
  char *mqtt_server;
  char *mqtt_server2 = MQTT_SERVER2;
  // char* mqtt_server2 = "broker.hivemq.com";
  char *user = "";
  char *passw = "";
  // ######################################

  // MQTT topics
  char _msgTopic[MaxTopicLength];
  char _groupTopic[MaxTopicLength];
  char _errorTopic[MaxTopicLength];
  char _deviceName[MaxTopicLength];
  char _availTopic[MaxTopicLength];
  char _stateTopic[MaxTopicLength];
  char _stateTopic2[MaxTopicLength];
  char _signalTopic[MaxTopicLength];

  char *topicArry[4] = {deviceTopic, _groupTopic, _availTopic, addGroupTopic};
  // ##############################################

  // MQTT connection flags
  // ######################

  // holds informamtion
  char bootTime[50];
  char bootErrors[150];
  bool firstRun = true;
  bool _failNTP = false;

  // ###################

  // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
  bool startWifi(char *ssid, char *password);
  bool startNTP();
  void start_clock();
  void network_looper();
  bool bootKeeper();
  void start_network_services();

  // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void startMQTT();
  bool subscribeMQTT();
  void createTopics();
  void callback(char *topic, byte *payload, unsigned int length);
  void msgSplitter(const char *msg_in, int max_msgSize, char *prefix, char *split_msg);
  void pub_offline_errs();
  void firstRun_ResetKeeper(char *msg);
  void register_err(char *inmsg);
  // ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
  void feedTheDog();
  void startWDT();
  // void acceptOTA();
};

#endif
