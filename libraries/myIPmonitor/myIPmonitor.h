#ifndef myIPmonitor_h
#define myIPmonitor_h
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <myLOG.h>

class IPmonitoring
{
#define MAXPING_TIME 20    /* Max time between pings, sec */
#define MINPING_TIME 10    /* Min time between pings, sec */
#define TIME_TO_MAXPING 30 /* After this time period ping will be every MAXPING_TIME sec */
#define RESET_BOOT_ERR 2   /* Time to wait until reset due to NO-internet or NO-NTP failure */
#define LOG_ENTRIES 15
#define ENTRY_LENGTH 15

    typedef bool (*cb_func)(char *externalSite, byte pings);

public:
    unsigned long bootClk = 0;
    unsigned long currentstateClk = 0;
    unsigned int dCounter = 0;
    // unsigned int dDuration = 0;
    bool isConnected = false;
    const char* libVer="IPmon_v0.1";

private:
    char *_IP;
    char *_nick;
    char *_conlog_filename;
    byte _adaptive_ping_val = MINPING_TIME;
    byte _pingCounter = 0;
    bool _needRESET = true;
    bool _firstPing = true;
    char _inline_param[2][12];
    unsigned long _lastCheck = 0;

    flashLOG _conFlog;
    cb_func _ping_cb;

public:
    IPmonitoring(char *IP, char *nick);
    ~IPmonitoring();
    void start(cb_func ping);
    void loop();
    void printFlog(int i = NULL);
    void getStatus(int h = 24);
    void deleteLOG();

private:
    char *_str_concat(const char *a, const char *b);
    void _disco_service();
    void _reco_service();
    bool _ping_client();
    void _ping_looper();
    bool _verify_internet_ok();
    void _post_msg(char *inmsg, char *inmsg2 = "");
    void _reset_bootFailure();
    void _conv_epoch(time_t t, char *retDate);
    void _conv_epoch_duration(long t1, long t2, char *clk);

    // ~~~~~~~~~~FlashLOGS
    void _LOGconnection();
    void _LOGdisconnection();
    void _readFlog_2row(flashLOG &LOG, int numLine, time_t &retTime, byte &retType);
    void _writeFlog_2row(flashLOG &LOG, byte Reason, time_t value = now(), bool writenow = false);
    int _inline_read(char *inputstr);
    bool _startFlogs();
    void _loopFlogs();
};

#endif