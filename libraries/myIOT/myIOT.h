#ifndef myIOT_h
#define myIOT_h

#include "Arduino.h"
typedef void (*cb_func)(char msg1[50]); // this define a generic functing with an input of 50 chars

class myIOT
{
public:
myIOT(char *devTopic);
void looper();
void startOTA();
void get_timeStamp();

void sendReset(char *header);
void start_services(cb_func funct, char *ssid="Xiaomi_D6C8", char *password="guyd5161", char *mqtt_user="guy", char *mqtt_passw="kupelu9e", char *mqtt_broker="192.168.3.200");
void pub_state(char *inmsg);
void pub_msg(char *inmsg);

bool useSerial = false;
bool useWDT = true;
bool useOTA = true;
bool extDefine = false; // must to set to true in order to use EXtMQTT

bool mqttConnected = 0;
char* deviceTopic = "";
const char *ver = "iot_1.40";

private:
char* ssid;
char* password;
cb_func ext_mqtt;


// time interval parameters
const int clockUpdateInt = (60*5);     // seconds to update NTP
const int WIFItimeOut = (1000 * 60) * 1/3;     // 20 sec try to connect WiFi
const int OTA_upload_interval = (1000 * 60) * 2;     // 2 minute to try OTA
const int time2Reset_noNetwork = (1000 * 60) * 5;     // minutues pass without any network
const int time2_tryReconnect = (1000 * 60) * 1/3;     // time between reconnection retries
volatile int wdtResetCounter = 0;
const int wdtMaxRetries = 20;     //seconds to bITE
long noNetwork_Counter = 0;     // clock
long OTAcounter = 0;     // clock
// ############################



//MQTT broker parameters
char* mqtt_server;
char* user = "";
char* passw = "";
// ######################################


// MQTT topics
char* msgTopic = "HomePi/Dvir/Messages";
char* groupTopic = "HomePi/Dvir/All";
char* deviceName="";
char* topicArry[2] = {deviceTopic, groupTopic};
char stateTopic[50];
char availTopic[50];
// ##############################################


// MQTT connection flags
int mqttFailCounter = 0;     // count tries to reconnect
int MQTTretries = 2;     // allowed tries to reconnect
// ######################


// holds informamtion
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
// ###################


// ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
void startNetwork(char *ssid, char *password);
void startNTP();

void start_clock();
int networkStatus();
void network_check();


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

#endif
