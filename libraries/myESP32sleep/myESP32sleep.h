#ifndef myESP32sleep_h
#define myESP32sleep_h

#include "Arduino.h"
#include "EEPROM.h"
#include "time.h"
#include "WiFi.h"
#include "secrets.h"
#include <PubSubClient.h>

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


    //MQTT broker parameters
    char *mqtt_server;
    // char *mqtt_server2 = MQTT_SERVER2;
    char *user = "";
    char *passw = "";

    cb_func _runFunc;

    WiFiClient espClient;
    PubSubClient mqttClient;

public:
    bool use_wifi = true;
    char *dev_name = "myESP32_devname";
    char *wifi_ssid;// = "WIFI_NETWORK_BY_USER";
    char *wifi_pass;// = "WIFI_PASSWORD_BY_USER";

    char sys_presets_str[100];
    char wake_sleep_str[150];

    const char *ver = "1.2";

private:
    // ~~~~~~~~ EEPROM ~~~~~~~~~~~~~
    int getEEPROMvalue(byte i = 0);
    void saveEEPROMvalue(int val, byte i = 0);
    void start_eeprom(byte i = 0);

    // ~~~~~~~~ Wifi & NTP ~~~~~~
    void startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 0, const char *ntpServer = "pool.ntp.org");
    bool startWifi();
    void getTime();
    void Avg_Array_zeroing();

    // ~~~~~~~ Sleep & Drift calcs ~~~
    void update_driftArray(float lastboot_drift);
    void driftUpdate(float drift_value, byte cell = 0);
    void new_driftUpdate(float drift_value, byte cell = 0);
    int calc_nominal_sleepTime();

    // void onConnectionEstablished();

public:
    esp32Sleep(int deepsleep = 30, int forcedwake = 15, char *devname = "ESP32device"); 
    bool startServices(char *ssid = SSID_ID, char *password = PASS_WIFI, char *mqtt_user = MQTT_USER, char *mqtt_passw = MQTT_PASS, char *mqtt_broker = MQTT_SERVER1);
    void sleepNOW(float sec2sleep = 2700);
    void check_awake_ontime(int min_t_avoidSleep = 10);
    void wait_forSleep();
    void run_func(cb_func cb);
};

#endif
