#ifndef myIOT_h
#define myIOT_h

#include "Arduino.h"
typedef void (*cb_func)(char msg1[50]); // this define a generic functing with an input of 50 chars

#define jfile "myfile.json"



class myIOT
{
public:
myIOT(char *devTopic);
void looper();
void startOTA();
void get_timeStamp();
void return_clock(char ret_tuple[20]);
void return_date(char ret_tuple[20]);

void sendReset(char *header);
void start_services(cb_func funct, char *ssid="Xiaomi_D6C8", char *password="guyd5161", char *mqtt_user="guy", char *mqtt_passw="kupelu9e", char *mqtt_broker="192.168.3.200");
void pub_state(char *inmsg);
void pub_msg(char *inmsg);
int inline_read(char *inputstr);

bool useSerial = false;
bool useWDT    = true;
bool useOTA    = true;
bool extDefine = false; // must to set to true in order to use EXtMQTT

// byte inline_param_amount = 2;
char inline_param[3][20]; //values from user

bool mqttConnected  = 0;
char* deviceTopic   = "";
const char *ver     = "iot_2.0";
char timeStamp[50];


bool resetBoot_flag  = false;
long updated_bootTime = 0;
int resetIntervals   = 30;

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
char* deviceName   = "";
char* topicArry[2] = {deviceTopic, groupTopic};
char stateTopic[50];
char availTopic[50];
// ##############################################


// MQTT connection flags
int mqttFailCounter = 0;     // count tries to reconnect
int MQTTretries     = 2;     // allowed tries to reconnect
// ######################


// holds informamtion
char msg[150];
char bootTime[50];
bool firstRun = true;
// ###################

long _savedBoot_Calc  = 0;
long _savedBoot_reset = 0;


// ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
void startNetwork(char *ssid, char *password);
void startNTP();
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

class FVars
{
public:
  FVars();
  void getValue();
  void setValue(char val[20]);

private:

};

#endif
