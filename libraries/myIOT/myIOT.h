#ifndef myIOT_h
#define myIOT_h

#include "Arduino.h"
typedef void (*cb_func)(char msg1[50]); // this define a generic functing with an input of 50 chars

#define jfile "myfile.json"

class FVars
{
public:
FVars(char* key="def_key");
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
const char* _key;

};

class myIOT
{
public:
myIOT(char *devTopic, char *key="failNTPcount");
void looper();
void startOTA();
void get_timeStamp(time_t t = 0);
void return_clock(char ret_tuple[20]);
void return_date(char ret_tuple[20]);

void sendReset(char *header);
void start_services(cb_func funct, char *ssid="Xiaomi_D6C8", char *password="guyd5161", char *mqtt_user="guy", char *mqtt_passw="kupelu9e", char *mqtt_broker="192.168.3.200");
void notifyOnline();
void pub_state(char *inmsg);
void pub_msg(char *inmsg);
void pub_err(char *inmsg);
int inline_read(char *inputstr);

bool useSerial = false;
bool useWDT    = true;
bool useOTA    = true;
bool extDefine = false; // must to set to true in order to use EXtMQTT
bool useResetKeeper = false;
bool resetFailNTP   = false;

char inline_param[3][20]; //values from user

bool mqttConnected  = 0;
char* deviceTopic   = "";
const char *ver     = "iot_3.1";
char timeStamp[20];


bool resetBoot_flag    = false;
byte mqtt_detect_reset = 2;
long updated_bootTime  = 0;
int resetIntervals     = 10;

private:
char* ssid;
char* password;
cb_func ext_mqtt;


// time interval parameters
const int clockUpdateInt        = 60 * 5;            // seconds to update NTP
const int WIFItimeOut           = (1000 * 60) * 1/3;     // 20 sec try to connect WiFi
const int OTA_upload_interval   = (1000 * 60) * 5;     // 5 minute to try OTA
const int time2Reset_noNetwork  = (1000 * 60) * 10;     // minutues pass without any network
const int time2_tryReconnect    = (1000 * 60) * 5;     // time between reconnection retries
volatile int wdtResetCounter    = 0;
const int wdtMaxRetries         = 20;     //seconds to bITE
long noNetwork_Counter          = 0;     // clock
long OTAcounter                 = 0;     // clock
long lastReconnectTry           = 0;
// ############################



//MQTT broker parameters
char* mqtt_server;
char* user = "";
char* passw = "";
// ######################################


// MQTT topics
char* msgTopic     = "HomePi/Dvir/Messages";
char* groupTopic   = "HomePi/Dvir/All";
char* errorTopic   = "HomePi/Dvir/Errors";
char* deviceName   = "HomePi/DvirHomePi/DvirHomePi/DvirHomePi/Dvir";
char* availTopic   = "HomePi/DvirHomePi/DvirHomePi/DvirHomePi/Dvir";

char stateTopic[50];
char* topicArry[4] = {deviceTopic, groupTopic, errorTopic, availTopic};

// ##############################################


// MQTT connection flags
int mqttFailCounter = 0;     // count tries to reconnect
int MQTTretries     = 2;     // allowed tries to reconnect
// ######################


// holds informamtion
char msg[150];
char bootTime[50];
bool firstRun = true;
bool _failNTP = false;
FVars _failNTPcounter_inFlash;
// ###################

long _savedBoot_Calc  = 0;
long _savedBoot_reset = 0;



// ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
void startNetwork(char *ssid, char *password);
bool startNTP();
void start_clock();
void networkStatus();
bool bootKeeper();


// ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void startMQTT();
int subscribeMQTT();
void createTopics(const char *devTopic, char *state, char *avail);
void callback(char* topic, byte* payload, unsigned int length);
void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg);
// ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
void feedTheDog();
void startWDT();
void acceptOTA();
};

class timeOUT
{
  #define _key1 "_endTO"
  #define _key2 "_codedTO"
  #define _key3 "_updatedTO"

private:
long _calc_endTO  = 0; // corrected clock ( case of restart)
long _savedTO     = 0; // clock to stop TO
bool _inTO        = false;
bool _onState     = false;
byte loopVerify   = 0;

public:
// long savedTO             = 0;   // clock to stop TO
int inCode_timeout_value = 0;   // default value for TO ( hard coded )


public:
timeOUT(char *key, int def_val, char *key2=_key2, char *key3=_key3);
bool looper();
int flashRead();
int remain();
bool begin(bool newReboot = true);
void restart_to();
void setNewTimeout(int to);
void convert_epoch2clock(long t1, long t2, char* time_str, char* days_str);
void endNow();
void updateTOinflash(int TO);
void restore_to();

FVars inCodeTimeOUT_inFlash;
FVars endTimeOUT_inFlash;
FVars updatedTimeOUT_inFlash;

private:
void switchON();
void switchOFF();

};

#endif
