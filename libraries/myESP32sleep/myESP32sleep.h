#ifndef myESP32sleep_h
#define myESP32sleep_h

#include "Arduino.h"
#include "EEPROM.h"
#include "secrets.h"

extern int bootCounter; //RTC var

class esp32Sleep
{
    typedef void (*cb_func)();
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define EEPROM_SIZE 16

// #define DEVICE_TOPIC "ESP32_SLEEP"
// #define MQTT_PREFIX "myHome"
// #define MQTT_GROUP ""

private:
    bool _use_extfunc = false;

    cb_func _runFunc;

public:
    char sys_presets_str[100];
    char wake_sleep_str[250];

    const char *ver = "ESP32_SLEEP_3.2";
    int no_sleep_minutes = 0;

    long startsleep_clock = 0;
    int sleepduration = 0;

    struct wakeStatus
    {
        int sleep_duration; //sec
        int awake_duration; 
        int bootCount;
        int wake_err;
        float drift_err;
        long startsleep_clock;
        long nextwake_clock;
        long wake_clock;
        char *name;
    };

    wakeStatus WakeStatus={0,0,0,0,0.0,0,0,0,"This is saved to any name"};

    struct tm timeinfo;
    time_t epoch_time;

private:
    // ~~~~~~~~ EEPROM ~~~~~~~~~~~~~
    int getEEPROMvalue(byte i = 0);
    void saveEEPROMvalue(int val, byte i = 0);
    void start_eeprom(byte i = 0);
    void Avg_Array_zeroing();

    // ~~~~~~~ Sleep & Drift calcs ~~~
    void update_driftArray(float lastboot_drift);
    void new_driftUpdate(float drift_value, byte cell = 0);
    int calc_nominal_sleepTime();//struct tm *timeinfo=0, time_t *epoch_time=0);
    void update_clock();
public:
    esp32Sleep(int deepsleep = 30, int forcedwake = 15, char *devname = "ESP32device");
    void run_func(cb_func cb);
    void startServices();//struct tm *timeinfo=0, time_t *epoch_time=0);
    void check_awake_ontime(int min_t_avoidSleep = 10);//struct tm *timeinfo, time_t *epoch_time, int min_t_avoidSleep = 10);
    void sleepNOW(float sec2sleep = 2700);
    void wait_forSleep(bool wifiOK = true, bool nosleep = false);//struct tm *timeinfo=0, time_t *epoch_time=0, bool wifiOK = true, bool nosleep = false);
};

#endif
