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
#define VER "sleepyESP_v0.8"
#define MINUTES2SEC 60
#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */

  typedef void (*cb_func)();

private:
#if isESP8266
  const float _driftFactor = 1.0435; /* ESP8266 error correction for wake time - adds time to nominal sleep time*/
#elif isESP32
  const float _driftFactor = 1.006; /* ESP32 error correction for wake time - adds time to nominal sleep time*/
#endif

  char _devname[15];
  bool _clkAlign = true;                /* Adujst sllep time that will be on round clk */
  bool _ready2Sleep = false;
  bool _force_postpone_sleep = false;

  uint8_t _deepsleep_mins = 0;
  uint8_t _nominal_wait_secs = 0;
  uint8_t _sec_to_wait_big_drift = 13;
  uint8_t _sec_to_ignore_wake_before_time = 30;

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
  uint8_t foce_postpone_sec = 120;
  bool clock_update_success = false;

public:
  sleepyESP();
  void start(uint8_t deepsleep_mins, uint8_t forcedwake_secs, char *devname, cb_func wake_cb = nullptr, cb_func sleep_cb = nullptr, bool clkAlign = true);
  void wait2Sleep();
  bool post_wake_clkUpdate();
  void delay_sleep(int sec_delay = 120);
  void update_sleep_duration(int sleep_time);

private:
  void _onWake_cb();
  void _calc_nextwakeClk();
  void _gotoSleep(int seconds2sleep = 10);
  // void _convert_epoch2clock(long t1, long t2, char *time_str, char *days_str);

  void EEPROMWritelong(int address, long value);
  long EEPROMReadlong(long address);
};

#endif
