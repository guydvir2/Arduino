#ifndef esp8266sleep_h
#define esp8266sleep_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <myIOT.h>

class esp8266Sleep
{
#define VER "ESPSleep_v0.2"
#define MINUTES 60
#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */
  typedef void (*cb_func)();

private:
  const float driftFactor = 1.048; /* ESP8266 error correction for wake time - adds time to nominal sleep time*/

  char _devname[30];
  int _deepsleep = 0;
  int _forcedWake = 0;
  int _sec_to_wait_big_drift = 13;
  int _sec_to_ignore_wake_before_time = 30;
  bool _start_ota = false;
  bool _ready2Sleep = false;
  unsigned long _ota_counter = 0;

  time_t wakeClock = 0;
  cb_func _wake_cb;
  cb_func _sleep_cb;

  FVars FVAR_bootClock;
  FVars FVAR_bootCounter;
  FVars FVAR_nextWakeClock;

public:
  int drift = 0;
  int bootCount = 0;
  int sec_waitOTA = 120;
  int totalSleepTime = 0;
  int nextsleep_duration = 0;
  bool clock_update_success = false;

public:
  esp8266Sleep();
  void start(int deepsleep, int forcedwake, char *devname, cb_func ext_cb, cb_func wake_cb=nullptr);
  bool wait2Sleep();
  void onWake_cb();
  bool after_wakeup_clockupdates();
  void init_OTA();

private:
  void gotoSleep(int seconds2sleep = 10);
  void nextSleepCalculation();
};

#endif
