#ifndef myESP32sleep_h
#define myESP32sleep_h

#include "Arduino.h"
#include "EEPROM.h"
#include "time.h"
#include "secrets.h"
#include <Ticker.h>

class esp32Sleep
{
    typedef void (*cb_func)();
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define EEPROM_SIZE 16

#define DEVICE_TOPIC "ESP32_SLEEP"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""

private:
    struct tm _timeinfo;
    time_t _epoch_time;
    int _deepsleep_time = 0;  // nominal minutes to sleep
    int _forcedwake_time = 0; // forced time to stay awake before sleep
    bool _wifi_status = false;
    bool _use_extfunc = false;
    char MQTTmsgtopic[50];
    char MQTTlogtopic[50];
    char MQTTavltopic[50];
    char MQTTdevtopic[50];
    
    char MQTTlastctopic[50];

    cb_func _runFunc;


public:
    char *dev_name = "myESP32_devname";

    char sys_presets_str[100];
    char wake_sleep_str[150];

    const char *ver = "2.1";
    Ticker wdt;

private:
    // ~~~~~~~~ EEPROM ~~~~~~~~~~~~~
    int getEEPROMvalue(byte i = 0);
    void saveEEPROMvalue(int val, byte i = 0);
    void start_eeprom(byte i = 0);

    void getTime();
    void Avg_Array_zeroing();

    // ~~~~~~~ Sleep & Drift calcs ~~~
    void update_driftArray(float lastboot_drift);
    void driftUpdate(float drift_value, byte cell = 0);
    void new_driftUpdate(float drift_value, byte cell = 0);
    int calc_nominal_sleepTime(struct tm *timeinfo, time_t *epoch_time);

public:
    esp32Sleep(int deepsleep = 30, int forcedwake = 15, char *devname = "ESP32device");
    bool startServices();
    void sleepNOW(float sec2sleep = 2700);
    void check_awake_ontime(int min_t_avoidSleep, struct tm *timeinfo, time_t *epoch_time);
    void wait_forSleep(struct tm *timeinfo, time_t *epoch_time, bool wifiOK= true);
    void run_func(cb_func cb);
};

#endif
