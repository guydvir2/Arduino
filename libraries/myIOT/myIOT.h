#ifndef myIOT_h
#define myIOT_h

#define jfile "myfile.json"

#include "secrets.h"
#include "Arduino.h"
#include <myJSON.h>


#include <ESP8266WiFi.h>
#include <PubSubClient.h> //MQTT
#include <Ticker.h>       //WDT
#include <NtpClientLib.h>

// OTA libraries
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Telegram libraries
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>


// define generic functiobs
typedef void (*cb_func)(char msg1[50]);
typedef void (*cb_func2)(String msg1, String msg2, String msg3, char *msg4);


class FVars {
public:
FVars(char* key="def_key", char* pref="");
bool getValue(int &ret_val);
bool getValue(long &ret_val);
bool getValue(char value[20]);

void setValue(int val);
void setValue(long val);
void setValue(char* val);

void remove();
void printFile();
void format();

private:
char _key[20];

};

class myTelegram {

private:
UniversalTelegramBot bot;
WiFiClientSecure client;
cb_func2 _ext_func;

private:
char _bot[100];
char _chatID[100];
char _ssid[20];
char _password[20];
int _Bot_mtbs = 10; //mean time between scan messages in millis

long _Bot_lasttime;   //last time messages' scan has been done

private:
void handleNewMessages(int numNewMessages);

public:
myTelegram(char* Bot, char* chatID, char* ssid = SSID_ID, char* password = PASS_WIFI int checkServer_interval = _Bot_mtbs);
void begin(cb_func2 funct);
void send_msg(char *msg);
void looper();
};

class myIOT {
static const int MaxTopicLength = 64;                           //topics

public:
WiFiClient espClient;
PubSubClient mqttClient;
Ticker wdt;

myIOT(char *devTopic, char *key="failNTPcount");
void start_services(cb_func funct, char *ssid=SSID_ID, char *password=PASS_WIFI, char *mqtt_user=MQTT_USER, char *mqtt_passw=MQTT_PASS, char *mqtt_broker="192.168.3.200");
void looper();
void startOTA();
// void startTelegram(cb_func2 funct2);
void get_timeStamp(time_t t = 0);
void return_clock(char ret_tuple[20]);
void return_date(char ret_tuple[20]);

void sendReset(char *header);
void notifyOnline();
void notifyOffline();
void pub_state(char *inmsg);
void pub_msg(char *inmsg);
void pub_err(char *inmsg);
int inline_read(char *inputstr);

// ~~~~~~ Services ~~~~~~~~~
bool useSerial      = false;
bool useWDT         = true;
bool useOTA         = true;
bool extDefine      = false;                           // must to set to true in order to use EXtMQTT
bool useResetKeeper = false;
bool resetFailNTP   = false;
bool useTelegram    = false;
// ~~~~~~~~~~~~~~~~~~~~~~~~~
char inline_param[6][20];                           //values from user

bool mqttConnected         = 0;
bool alternativeMQTTserver = false;
bool is_online             = false;
byte mqtt_detect_reset     = 2;
char prefixTopic  [MaxTopicLength];
char deviceTopic  [MaxTopicLength];
char addGroupTopic[MaxTopicLength];

const char *ver     = "iot_5.3";
char timeStamp[20];
long updated_bootTime  = 0;
int resetIntervals     = 10;


private:
char* ssid;
char* password;
cb_func ext_mqtt;
cb_func2 ext_telegram;


// time interval parameters
const int clockUpdateInt        = 60 * 60 * 5;                                      // seconds to update NTP
const int WIFItimeOut           = (1000 * 60) * 1/3;                               // 20 sec try to connect WiFi
const int OTA_upload_interval   = (1000 * 60) * 5;                               // 5 minute to try OTA
const long time2Reset_noNetwork = (1000 * 60) * 1;                               // minutues pass without any network
const int time2_tryReconnect    = (1000 * 60) * 3;                               // time between reconnection retries
volatile int wdtResetCounter    = 0;
const int wdtMaxRetries         = 30;                               //seconds to bITE
long noNetwork_Clock            = 0;                               // clock
long allowOTA_clock             = 0;                               // clock
long lastReconnectTry           = 0;
// ############################


//MQTT broker parameters
char* mqtt_server;
// char* mqtt_server2 = "iot.eclipse.org";
char* mqtt_server2 = "broker.hivemq.com";
char* user  = "";
char* passw = "";
// ######################################


// MQTT topics
char _msgTopic  [MaxTopicLength];
char _groupTopic[MaxTopicLength];
char _errorTopic[MaxTopicLength];
char _deviceName[MaxTopicLength];
char _availTopic[MaxTopicLength];
char _stateTopic[MaxTopicLength];

char* topicArry[4] = {deviceTopic, _groupTopic, _availTopic, addGroupTopic};
// ##############################################


// MQTT connection flags
int mqttFailCounter = 0;                               // count tries to reconnect
int MQTTretries     = 0;                               // allowed tries to reconnect
// ######################


// holds informamtion
char bootTime[50];
char bootErrors [150];
bool firstRun = true;
bool _failNTP = false;
FVars _failNTPcounter_inFlash;
// ###################

// ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
void startNetwork(char *ssid, char *password);
bool startNTP();
void start_clock();
void networkStatus();
bool bootKeeper();


// ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void startMQTT();
int subscribeMQTT();
void createTopics();
void callback(char* topic, byte* payload, unsigned int length);
void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg);
void publish_errs();
// ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
void feedTheDog();
void startWDT();
void acceptOTA();

};

class timeOUT {
  #define _key1 "_endTO"
  #define _key2 "_codedTO"
  #define _key3 "_updatedTO"
  #define _key4 "_startTO"

private:
long _calc_endTO  = 0; // corrected clock ( case of restart)
int _calc_TO      = 0; // stores UpdateTO or inCodeTO
bool _inTO        = false;

struct dTO {
        int on[3];
        int off[3];
        bool flag;
        bool useFlash;
        bool onNow;

};

const int items_each_array[3] = {3, 3, 1};

dTO defaultVals = {{0, 0, 0}, {0, 0, 59}, 1, 0, 0};


public:
int inCodeTO           = 0;   // default value for TO ( hard coded )
int updatedTO_inFlash  = 0;
long endTO_inFlash     = 0; // clock to stop TO
long startTO_inFlash   = 0; // clock TO started
dTO dailyTO            = {{19, 15, 0}, {20, 37, 0}, 1, 0, 0};
dTO dailyTO2           = {{1, 1, 0}, {17, 14, 0}, 1, 0, 0};
dTO *dTOlist[2]        = {&dailyTO, &dailyTO2};
const char *clock_fields[4] = {"ontime", "off_time", "flag", "use_inFl_vals"};


public:
timeOUT(char* key, int def_val);
bool looper();
int remain();
bool begin(bool newReboot = true);
void restart_to();
void setNewTimeout(int to, bool mins = true);
void convert_epoch2clock(long t1, long t2, char* time_str, char* days_str);
void endNow();
void updateTOinflash(int TO);
void restore_to();
void switchOFF();
long getStart_to();
void updateStart(long clock);

int calc_dailyTO(dTO &dailyTO);
void dailyTO_looper(dTO &dailyTO);
void check_dailyTO_inFlash(dTO &dailyTO, int x);
void store_dailyTO_inFlash(dTO &dailyTO, int x);
void restart_dailyTO (dTO &dailyTO);

FVars inCodeTimeOUT_inFlash;
FVars endTimeOUT_inFlash;
FVars updatedTimeOUT_inFlash;
FVars startTimeOUT_inFlash;
myJSON dailyTO_inFlash;

private:
void switchON();

};

#endif
