#ifndef sleepyESP_h
#define sleepyESP_h

#include <Arduino.h>
#include <EEPROM.h>

#if defined(ESP32)
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#endif

class sleepyESP
{
#define MINUTES2SEC 60
#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */

#define EEPROM_TOTAL 32
#define EEPROM_START_ADDRESS 0
#define _bootClock_addr EEPROM_START_ADDRESS
#define _bootCounter_addr EEPROM_START_ADDRESS + 4
#define _nextWake_clock_addr EEPROM_START_ADDRESS + 8
#define _allowed_wake_err_sec 15

  typedef void (*cb_func)();

private:
#if defined(ESP8266)
  const float _driftFactor = 1.05629; /* ESP8266 error correction for wake time - adds time to nominal sleep time*/
#elif defined(ESP32)
  const float _driftFactor = 1.0074; /* ESP32 error correction for wake time - adds time to nominal sleep time*/
#endif

  bool _clkAlign = true; /* Adujst sleep time that will be on round clk */
  bool _force_postpone_sleep = false;
  uint8_t _deepsleep_mins = 0;
  uint8_t _nominal_wait_secs = 15; /* total wake time, including connect WiFi and MQTT (on ESP8266 can easly be 11 sec, on ESP32 around 6sec) */
  unsigned long _force_postpone_millis = 0;

  cb_func _wake_cb;
  cb_func _sleep_cb;

public:
  int bootCount = 0;
  int totalSleepTime = 0;
  int wake_up_drift_sec = 0;
  int nextsleep_duration = 0;
  uint8_t force_postpone_sec = 120;
  bool clock_update_success = false;
  bool startSleepNOW = false; // Altough there is a timeout, you can ask to sleep NOW
  const char *VER = "sleepyESP_v1.0";

public:
  sleepyESP();
  void wait2Sleep();
  bool post_wake_clkUpdate();
  void delay_sleep(int sec_delay = 120);
  void update_sleep_duration(int sleep_time);
  void start(const uint8_t &deepsleep_mins, const uint8_t &forcedwake_secs, cb_func wake_cb = nullptr, cb_func sleep_cb = nullptr, bool clkAlign = true);

private:
  void _onWake_cb();
  void _calc_nextSleep_duration();
  void _gotoSleep(int seconds2sleep = 10);
  void _saveNext_wakeup(int duration, time_t t = time(nullptr));
  bool _checkClockSync();

  void EEPROMWritelong(int address, long value);
  long EEPROMReadlong(long address);
};

#endif
