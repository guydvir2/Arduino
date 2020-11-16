#ifndef myIOT_h
#define myIOT_h

#include <Arduino.h>
#include "secretsIOT8266.h"
#include <myJSON.h>
#include <myLOG.h>
#include <EEPROM.h>

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

#include <TimeLib.h>

// define generic functiobs
typedef void (*cb_func)(char msg1[50]);
typedef void (*cb_func2)(String msg1, String msg2, String msg3, char *msg4);

class FVars
{
public:
    FVars(char *key = "def_key", char *fname = "/myfile.json", bool _counter = false);
    bool getValue(int &ret_val);
    bool getValue(long &ret_val);
    bool getValue(char value[20]);

    void setValue(int val);
    void setValue(long val);
    void setValue(char *val);

    void remove();
    void printFile();
    void format();

private:
    char _key[20];
    myJSON json;
    static int _counter;
};

class myTelegram
{

private:
    UniversalTelegramBot bot;
    WiFiClientSecure client;
    cb_func2 _ext_func;

private:
    char _bot[100];
    char _chatID[100];
    char _ssid[20];
    char _password[20];
    int _Bot_mtbs = 1000; //mean time between scan messages in sec
#define def_time_check_M 5

    long _Bot_lasttime = 0; //last time messages' scan has been done

private:
    void handleNewMessages(int numNewMessages);

public:
    myTelegram(char *Bot, char *chatID, int checkServer_interval = def_time_check_M, char *ssid = SSID_ID, char *password = PASS_WIFI);
    void begin(cb_func2 funct);
    void send_msg(char *msg);
    void send_msg2(String msg);
    void looper();
};

class myIOT
{

public:
    WiFiClient espClient;
    PubSubClient mqttClient;
    Ticker wdt;
    flashLOG flog;

    myIOT(char *key = "failNTPcount");
    void start_services(cb_func funct, char *ssid = SSID_ID, char *password = PASS_WIFI, char *mqtt_user = MQTT_USER, char *mqtt_passw = MQTT_PASS, char *mqtt_broker = MQTT_SERVER1, int log_ents = 50, int log_len = 250);
    void looper();
    void startOTA();
    void get_timeStamp(time_t t = 0);
    void return_clock(char ret_tuple[20]);
    void return_date(char ret_tuple[20]);
    bool checkInternet(char *externalSite = "www.google.com", byte pings = 1);

    void sendReset(char *header);
    void notifyOnline();
    void notifyOffline();
    void pub_state(char *inmsg, byte i = 0);
    void pub_msg(char *inmsg);
    void pub_log(char *inmsg);
    void pub_ext(char *inmsg, char *name = "", bool retain = false);
    void pub_debug(char *inmsg);
    int inline_read(char *inputstr);

    // ~~~~~~ Flash Parameters~~~~~~~~
    bool read_fPars(char *filename, String &defs, JsonDocument &DOC, int JSIZE = 500);
    char *export_fPars(char *filename, JsonDocument &DOC, int JSIZE = 500);

    // ~~~~~~ Services ~~~~~~~~~
    bool useSerial = false;
    bool useWDT = true;
    bool useOTA = true;
    bool extDefine = false; // must to set to true in order to use EXtMQTT
    bool useResetKeeper = false;
    bool resetFailNTP = false;
    bool useextTopic = false;
    bool useNetworkReset = true; // allow reset due to no-network timeout
    bool useDebug = false;
    byte debug_level = 0; // 0- All, 1- system states; 2- log only
    // ~~~~~~~~~~~~~~~~~~~~~~~~~

    static const byte num_param = 6;
    char inline_param[num_param][100]; //values from user

    bool alternativeMQTTserver = false;
    bool NTP_OK = false;
    byte mqtt_detect_reset = 2;
    int noNetwork_reset = 30; // minutes

    static const int MaxTopicLength = 64; //topics
    char prefixTopic[MaxTopicLength];
    char deviceTopic[MaxTopicLength];
    char addGroupTopic[MaxTopicLength];
    char extTopic[MaxTopicLength];
    char mqqt_ext_buffer[3][150];

    const char *ver = "iot_8.9";
    char timeStamp[20];
    char *myIOT_paramfile = "/myIOT_param.json";

private:
    char *Ssid;
    char *Password;
    cb_func ext_mqtt;

    // time interval parameters
    const int clockUpdateInt = 60 * 60 * 5;           // seconds to update NTP
    const int WIFItimeOut = (1000 * 60) * 1 / 4;      // 30 sec try to connect WiFi
    const int OTA_upload_interval = (1000 * 60) * 10; // 10 minute to try OTA
    const int retryConnectWiFi = (1000 * 60) * 1;     // 5 minuter between fail Wifi reconnect reties

    long time2Reset_noNetwork = (1000 * 60L) * noNetwork_reset; // minutues pass without any network
    volatile int wdtResetCounter = 0;
    const int wdtMaxRetries = 60; //seconds to bITE
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

    const int _maxMQTTmsg = 180;
    const int _maxMQTTheader = 70;

    // ######################################

    // MQTT topics
    char _msgTopic[MaxTopicLength];
    char _groupTopic[MaxTopicLength];
    char _logTopic[MaxTopicLength];
    char _deviceName[MaxTopicLength];
    char _availTopic[MaxTopicLength];
    char _stateTopic[MaxTopicLength];
    char _stateTopic2[MaxTopicLength];
    char _signalTopic[MaxTopicLength];
    char _debugTopic[MaxTopicLength];

    char *topicArry[4] = {_deviceName, _groupTopic, _availTopic, addGroupTopic};
    // ##############################################

    // MQTT connection flags
    // ######################

    // holds informamtion
    char bootTime[50];
    bool firstRun = true;
    bool _failNTP = false;

    FVars _failNTPcounter_inFlash;
    // ###################

    // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
    bool startWifi(char *ssid, char *password);
    bool startNTP();
    void start_clock();
    bool network_looper();
    void start_network_services();

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void startMQTT();
    bool subscribeMQTT();
    void createTopics();
    void callback(char *topic, byte *payload, unsigned int length);
    void msgSplitter(const char *msg_in, int max_msgSize, char *prefix, char *split_msg);
    void firstRun_ResetKeeper(char *msg);
    void write_log(char *inmsg, int x);
    void _pub_generic(char *topic, char *inmsg, bool retain = false, char *devname = "");

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
#define _key4 "_startTO"

private:
    long _calc_endTO = 0; // corrected clock ( case of restart)
    int _calc_TO = 0;     // stores UpdateTO or inCodeTO
    bool _inTO = false;
    static int _ins_counter;

    struct dTO
    {
        int on[3];
        int off[3];
        bool flag;
        bool useFlash;
        bool onNow;
    };

    const int items_each_array[3] = {3, 3, 1};

    dTO defaultVals = {{0, 0, 0}, {0, 0, 59}, 1, 0, 0};

public:
    int inCodeTO = 0; // default value for TO ( hard coded )
    int updatedTO_inFlash = 0;
    long endTO_inFlash = 0;   // clock to stop TO
    long startTO_inFlash = 0; // clock TO started
    dTO dailyTO = {{22, 16, 0}, {22, 16, 10}, 1, 0, 0};
    const char *clock_fields[4] = {"ontime", "off_time", "flag", "use_inFl_vals"};
    char dTO_pubMsg[40];

public:
    timeOUT(char *key = "timeOUTsw");
    void set_fvars(int def_val = 60);
    bool looper();
    int remain();
    bool begin();
    void restart_to();
    void setNewTimeout(int to, bool mins = true);
    void convert_epoch2clock(long t1, long t2, char *time_str, char *days_str);
    void endNow();
    void updateTOinflash(int TO);
    void restore_to();
    void switchOFF();
    long getStart_to();
    void updateStart(long clock);

    int calc_dailyTO(dTO &dailyTO);
    void dailyTO_looper(dTO &dailyTO);
    void check_dailyTO_inFlash(dTO &dailyTO, int x = 0);
    void store_dailyTO_inFlash(dTO &dailyTO, int x = 0);
    void restart_dailyTO(dTO &dailyTO);

    FVars inCodeTimeOUT_inFlash;
    FVars endTimeOUT_inFlash;
    FVars updatedTimeOUT_inFlash;
    FVars startTimeOUT_inFlash;
    myJSON dailyTO_inFlash;

private:
    void switchON();
};
class hardReboot
{
private:
    struct eeproms_storage
    {
        byte cell_index;
        byte value;
    };
    eeproms_storage boot_Counter = {0, 0};
    int counter = 0;
    static int _counter;

public:
    hardReboot(int romsize = 64);
    void zero_cell();
    byte return_val();
    void print_val();
    bool check_boot(byte threshold = 3);
    bool resetFlag = false;
};

class mySwitch
{
#define PWM_RES 1024
#define SwitchOn LOW
#define RelayOn HIGH

    typedef void (*cb_func)();

private:
    static int _counter;
    int _switchPin;
    char _switchName[20];
    char _outMQTTmsg[150];
    char _out2MQTTmsg[150];
    char _outMQTTlog[150];
    char _outMQTTstate[20];
    bool _ext_det = HIGH;
    bool _retrig = false;
    char *_trig_name = "ext_trigger";
    char *_clockAlias = "Daily TimeOut";
    unsigned long _safetyOff_clock = 0;

private:
    void _checkSwitch_Pressed(int swPin, bool momentary = true);
    void _TOlooper(int det_reset);
    void _start_dailyTO();
    void _extTrig_looper();
    void _safetyOff();
    void _afterBoot_behaviour(int rebootState = -1);
    cb_func _swOn;
    cb_func _swOff;

public:
    const char *ver = "mySwitch_2.0";
    bool usePWM = false;
    bool useSerial = false;
    bool useInput = false;
    bool badBoot = false;
    bool usetimeOUT = true;
    bool useDailyTO = false;
    bool useEXTtrigger = false;
    bool usesafetyOff = false;
    bool usequickON = false;
    bool useHardReboot = false;
    bool onAt_boot = true;
    bool ext_trig_signal;
    bool is_momentery = true;
    bool last_relayState = false;
    bool trig_lastState = false;
    bool inputState = HIGH;
    bool _check_afterBoot = true;

    int inputPin = -1;
    float step_power = 0.2;
    float max_power = 1.0;
    float min_power = step_power;
    float def_power = 0.7;
    float current_power = 0.0;
    int START_dailyTO[3] = {23, 27, 30};
    int END_dailyTO[3] = {23, 28, 0};
    int set_safetyoff = 360; //minutes

    timeOUT TOswitch;
    hardReboot hReboot;

public:
    mySwitch();
    void config(int switchPin, int timeout_val = 60, char *name = "mySwitch");
    void changePower(float val);
    void switchIt(char *txt1, float state, bool ignoreTO = false);
    void begin();
    void looper(int det_reset);
    void extTrig_cb(bool det = HIGH, bool retrig = false, char *trig_name = "ext_trigger");
    bool postMessages(char outmsg[150], byte &msg_type);
    void adHOC_timeout(int mins, bool inMinutes = true);
    void setdailyTO(const int start_clk[], const int end_clk[]);
    void getMQTT(char *parm1, int p2, int p3, int p4);
    void all_off(char *from);
    void quickPwrON();
    void on_cb(cb_func On_cb = nullptr);
    void off_cb(cb_func Off_cb = nullptr);
};

#endif
