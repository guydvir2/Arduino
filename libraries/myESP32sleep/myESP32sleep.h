#ifndef myESP32sleep_h
#define myESP32sleep_h

#include "Arduino.h"
#include "EEPROM.h"
#include "secrets.h"

class esp32Sleep
{
    typedef void (*cb_func)();
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define EEPROM_SIZE 16

#define DEVICE_TOPIC "ESP32_SLEEP"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""

private:
    int _deepsleep_time = 0;  // nominal minutes to sleep
    int _forcedwake_time = 0; // forced time to stay awake before sleep
    bool _use_extfunc = false;

    cb_func _runFunc;

public:
    char *dev_name = "myESP32_devname";
    char sys_presets_str[100];
    char wake_sleep_str[150];

    const char *ver = "3.0";
    int no_sleep_minutes = 0;

    long startsleep_clock = 0;
    long nextwake_clock = 0;
    int sleepduration = 0;

private:
    // ~~~~~~~~ EEPROM ~~~~~~~~~~~~~
    int getEEPROMvalue(byte i = 0);
    void saveEEPROMvalue(int val, byte i = 0);
    void start_eeprom(byte i = 0);
    void Avg_Array_zeroing();

    // ~~~~~~~ Sleep & Drift calcs ~~~
    void update_driftArray(float lastboot_drift);
    void new_driftUpdate(float drift_value, byte cell = 0);
    int calc_nominal_sleepTime(struct tm *timeinfo, time_t *epoch_time);

public:
    esp32Sleep(int deepsleep = 30, int forcedwake = 15, char *devname = "ESP32device");
    void run_func(cb_func cb);
    void startServices(struct tm *timeinfo, time_t *epoch_time);
    void check_awake_ontime(struct tm *timeinfo, time_t *epoch_time, int min_t_avoidSleep = 10);
    void sleepNOW(float sec2sleep = 2700);
    void wait_forSleep(struct tm *timeinfo, time_t *epoch_time, bool wifiOK = true, bool nosleep = false);
};

#endif
