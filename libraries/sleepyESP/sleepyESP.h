#ifndef sleepyESP_h
#define sleepyESP_h

#include <Arduino.h>
#include <EEPROM.h>

#if defined(ARDUINO_ARCH_ESP8266)
#define isESP32 false
#define isESP8266 true
#elif defined(ESP32)
#define isESP32 true
#define isESP8266 false
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#endif

class sleepyESP
{
#define MINUTES2SEC 60
#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */

  typedef void (*cb_func)();

private:
#if isESP8266
  const float _driftFactor = 1.05629; /* ESP8266 error correction for wake time - adds time to nominal sleep time*/
#elif isESP32
  const float _driftFactor = 1.0074; /* ESP32 error correction for wake time - adds time to nominal sleep time*/
#endif

  char *_devname;
  bool _clkAlign = true; /* Adujst sllep time that will be on round clk */
  bool _force_postpone_sleep = false;

  uint8_t _deepsleep_mins = 0;
  uint8_t _nominal_wait_secs = 5;
  uint8_t _sec_to_wait_big_drift = 2;
  uint8_t _allowed_wake_err_sec = 15;

  const uint8_t _bootClock_addr = 0;
  const uint8_t _bootCounter_addr = 4;
  const uint8_t _nextWake_clock_addr = 8;

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
  const char *VER = "sleepyESP_v0.9";

public:
  sleepyESP();
  void start(const uint8_t &deepsleep_mins, const uint8_t &forcedwake_secs, char *devname, cb_func wake_cb = nullptr, cb_func sleep_cb = nullptr, bool clkAlign = true);
  void wait2Sleep();
  bool post_wake_clkUpdate();
  void delay_sleep(int sec_delay = 120);
  void update_sleep_duration(int sleep_time);

private:
  void _onWake_cb();
  void _calc_nextSleep_duration();
  void _gotoSleep(int seconds2sleep = 10);
  void _saveNext_wakeup(int duration,time_t t=time(nullptr));

  void EEPROMWritelong(int address, long value);
  long EEPROMReadlong(long address);
};

#endif
