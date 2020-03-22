#ifndef myESP32sleep_h
#define myESP32sleep_h

#include "Arduino.h"
#include "EEPROM.h"
#include "time.h"
#include "WiFi.h"



class esp32Sleep
{
    typedef void (*cb_func)();
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define EEPROM_SIZE 16

private:
    struct tm _timeinfo;
    time_t _epoch_time;
    int _deepsleep_time = 0;  // nominal minutes to sleep
    int _forcedwake_time = 0; // forced time to stay awake before sleep
    bool _wifi_status = false;
    bool _use_extfunc = false;

    cb_func _runFunc;

public:
    bool use_wifi = false;
    char *dev_name = "myESP32_devname";
    char *wifi_ssid = "WIFI_NETWORK_BY_USER";
    char *wifi_pass = "WIFI_PASSWORD_BY_USER";
    char sleepstr[250];

private:
    // ~~~~~~~~ EEPROM ~~~~~~~~~~~~~
    int getEEPROMvalue(byte i = 0);
    void saveEEPROMvalue(int val, byte i = 0);
    void start_eeprom(byte i = 0);

    // ~~~~~~~~ Wifi & NTP ~~~~~~
    void startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 0, const char *ntpServer = "pool.ntp.org");
    bool startWifi();
    bool getTime();
    void Avg_Array_zeroing();
    // ~~~~~~~ Sleep & Drift calcs ~~~
    void driftUpdate(float drift_value, byte cell = 0, byte update_freq = 10);
    int calc_nominal_sleepTime();

public:
    esp32Sleep(int deepsleep = 30, int forcedwake = 15, char *devname = "ESP32device");
    bool startServices();
    void printClock();
    void sleepNOW(float sec2sleep = 2700);
    void check_awake_ontime(int min_t_avoidSleep = 10);
    void wait_forSleep();
    void printUpdatedClock(char *hdr = "");
    void run_func(cb_func cb);
};

#endif
