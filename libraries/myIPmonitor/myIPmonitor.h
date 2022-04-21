#ifndef myIPmonitor_h
#define myIPmonitor_h

#if defined(ARDUINO_ARCH_ESP8266)
#define isESP8266 true
#define isESP32 false
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#define isESP32 true
#define isESP8266 false
#include <WiFi.h>
#else
#error Architecture unrecognized by this code.
#endif

#include <myLOG.h>

class IPmonitoring
{
#define MAXPING_TIME 30  /* Max time between pings, sec */
#define MINPING_TIME 10  /* Min time between pings, sec */
#define RESET_BOOT_ERR 2 /* Time to wait until reset due to NO-internet or NO-NTP failure */
#define LOG_ENTRIES 200   /* Entries saved in LOG */
#define ENTRY_LENGTH 15  /* Length of each entry - 12 chars */

    typedef bool (*cb_func)(char *externalSite, uint8_t pings);

public:
    char *nick;
    time_t currentstateClk = 0;
    int dCounter = 0;
    bool connState = false;
    const char *libVer = "NETmon_v0.3";

private:
    char *_IP;
    char *_conlog_filename;
    char _inline_param[2][12];
    uint8_t _adaptive_ping_val = MINPING_TIME;
    uint8_t _pingCounter = 0;

    bool _needRESET = true;
    bool _firstPing = true;
    bool _msgOUT = false;
    time_t reset_delay = 0;
    time_t _lastCheck = 0;

    flashLOG _ConnectLOG;
    cb_func _ping_cb;
    cb_func _msgout_cb;

public:
    IPmonitoring(char *IP, char *nick);
    ~IPmonitoring();
    void start(cb_func ping, cb_func outmsg = NULL);
    void loop();
    void printFlog(int i = 0);
    void getStatus(int h = 24);
    void deleteLOG();
    void enter_fake_LOGentry(time_t t, uint8_t reason);
    bool get_msg(char retMSG[]);
    void clear_buffer();

private:
    char *_str_concat(const char *a, const char *b);
    void _disconnect_cb();
    void _reconnect_cb();
    bool _ping_client();
    void _ping_looper();
    bool _boot_chk_internet_ntp();
    void _reset_bootFailure();
    void _conv_epoch(time_t &t, char *retDate);
    void _conv_epoch_duration(long t1, long t2, char *clk);
    void _post_msg(char *inmsg, uint8_t msg_type = 0);
    void _postExtMsg(char *inmsg);

    // ~~~~~~~~~~FlashLOGS
    void _LOGconnection();
    void _LOGdisconnection();
    void _readLOG(flashLOG &LOG, int numLine, time_t &retTime, uint8_t &retType);
    void _writeLOG(flashLOG &LOG, uint8_t Reason, time_t value, bool writenow = false);
    uint8_t _inline_read(char *inputstr);
    bool _startFlogs();
    void _loopFlogs();
};

#endif