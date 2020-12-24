#ifndef esp8266sleep_h
#define esp8266sleep_h

#include <Arduino.h>
#include <EEPROM.h>
#include <TimeLib.h>

#if defined(ARDUINO_ARCH_ESP8266)
#define isESP32 false
#define isESP8266 true
#elif defined(ESP32)
#define isESP32 true
#define isESP8266 false
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#endif

class esp8266Sleep
{
#define VER "ESPSleep_v0.7"
#define MINUTES 60
#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */

  typedef void (*cb_func)();

private:
#if isESP8266
  const float driftFactor = 1.0435; /* ESP8266 error correction for wake time - adds time to nominal sleep time*/
#elif isESP32
  const float driftFactor = 1.006; /* ESP32 error correction for wake time - adds time to nominal sleep time*/
#endif

  char _devname[30];
  int _deepsleep = 0;
  int _forcedWake = 0;
  int _sec_to_wait_big_drift = 13;
  int _sec_to_ignore_wake_before_time = 30;
  bool _start_delay = false;
  bool _ready2Sleep = false;
  unsigned long _delay_counter = 0;
  const byte _bootClock_addr = 0;
  const byte _bootCounter_addr = 4;
  const byte _nextWake_clock_addr = 8;

  time_t wakeClock = 0;
  cb_func _wake_cb;
  cb_func _sleep_cb;

public:
  int drift = 0;
  int bootCount = 0;
  int sec_wait = 120;
  int totalSleepTime = 0;
  int nextsleep_duration = 0;
  bool clock_update_success = false;

public:
  esp8266Sleep();
  void start(int deepsleep, int forcedwake, char *devname, cb_func wake_cb = nullptr, cb_func sleep_cb = nullptr);
  void wait2Sleep();
  bool after_wakeup_clockupdates();
  void delay_sleep(int sec_delay = 120);
  void update_sleep_duration(int sleep_time);

private:
  void onWake_cb();
  void nextSleepCalculation();
  void gotoSleep(int seconds2sleep = 10);
  void EEPROMWritelong(int address, long value);
  long EEPROMReadlong(long address);
};

#endif
