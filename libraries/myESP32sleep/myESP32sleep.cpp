#include "Arduino.h"
#include "myESP32sleep.h"

#define drift_ArraySize 3
#define zeroval 999.0

RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;
RTC_DATA_ATTR float driftRTC = 0;
RTC_DATA_ATTR long clock_beforeSleep = 0;
RTC_DATA_ATTR float driftsArray_RTC[drift_ArraySize];

// ~~~~~~~~ Start ~~~~~~~~~
esp32Sleep::esp32Sleep(int deepsleep, int forcedwake, char *devname)
{
  WakeStatus.sleep_duration = deepsleep;
  WakeStatus.awake_duration = forcedwake;
  WakeStatus.name = devname;
}
void esp32Sleep::startServices()
{
  start_eeprom();
  Avg_Array_zeroing();
  check_awake_ontime();
}
void esp32Sleep::run_func(cb_func cb)
{
  _runFunc = cb;
  _use_extfunc = true;
}

// ~~~~~~~ EEPROM ~~~~~~~~~
int esp32Sleep::getEEPROMvalue(byte i)
{
  int eeprom_drift = EEPROM.read(i) * pow(-1, EEPROM.read(i + 1));
  return eeprom_drift;
}
void esp32Sleep::saveEEPROMvalue(int val, byte i)
{
  EEPROM.write(i, abs(val));
  if (val < 0)
  {
    EEPROM.write(i + 1, 1);
    EEPROM.commit();
  }
  else
  {
    EEPROM.write(i + 1, 2);
    EEPROM.commit();
  }
}
void esp32Sleep::start_eeprom(byte i)
{
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    if (debug_mode)
    {
      Serial.println("Fail to load EEPROM");
    }
  }
}
void esp32Sleep::Avg_Array_zeroing()
{
  if (bootCounter == 0)
  {
    for (int a = 0; a < drift_ArraySize; a++)
    {
      driftsArray_RTC[a] = zeroval;
    }
    if (debug_mode)
    {
      Serial.println("Erasing EEPROM");
    }
  }
}

// ~~~~~~~ Sleep & Drift calcs ~~~
void esp32Sleep::update_clock()
{
  time(&epoch_time);
  getLocalTime(&timeinfo);
}
void esp32Sleep::check_awake_ontime(int min_t_avoidSleep) 
{
  update_clock();
  bootCounter++;
  WakeStatus.bootCount = bootCounter;
  sprintf(sys_presets_str, "deviceName:[%s]; SleepTime: [%d min]; Forced-aWakeTime: [%d sec]", WakeStatus.name, WakeStatus.sleep_duration, WakeStatus.awake_duration);
  sprintf(wake_sleep_str, "Boot#: [%d]; Wake_Clock: [%02d:%02d:%02d]; ", bootCounter, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // ~~~~ Check if clock is OK ~~~
  if (timeinfo.tm_year >= 120)
  {
    if (clock_beforeSleep > 0)
    {
      int wake_diff = (long)epoch_time - clock_expectedWake;           // not first boot
      WakeStatus.wake_err = wake_diff - (int)(round(millis() / 1000)); // diff between calc wake clock and current time
      WakeStatus.wake_clock = (long)epoch_time;

      char tt[200];
      sprintf(tt, "Woke_after: [%d sec]; wake_Drift: [%d sec]; ", (long)epoch_time - clock_beforeSleep, WakeStatus.wake_err);
      strcat(wake_sleep_str, tt);
      new_driftUpdate(WakeStatus.wake_err, 0);

      // ~~~~~~~ waking up before time expected ~~~~
      if (WakeStatus.wake_err < 0 && wake_diff < 0)
      {
        if (abs(wake_diff) <= min_t_avoidSleep)
        {
          // wake SHORT while before time, wait using a delay func.
          sprintf(tt, "Pause_wakeBeforeTime: [%d sec]; ", abs(wake_diff));
          strcat(wake_sleep_str, tt);
          delay(1000 * abs(wake_diff));
        }
        else
        {
          // wake before time, goto sleep.
          sleepNOW(abs(wake_diff));
        }
      }
    }
    else if (getEEPROMvalue() > 0)
    {
      driftRTC = (int)getEEPROMvalue();
      WakeStatus.drift_err = driftRTC;
    }
    else
    {
      saveEEPROMvalue(0, 0);
    }
  }

  Serial.println(sys_presets_str);
  Serial.println(wake_sleep_str);
}
int esp32Sleep::calc_nominal_sleepTime()
{
  update_clock();
  int nominal_nextSleep = 0;
  char tt[150];

  if (timeinfo.tm_year >= 120)
  {
    nominal_nextSleep = WakeStatus.sleep_duration * 60 - (timeinfo.tm_min * 60 + timeinfo.tm_sec) % (WakeStatus.sleep_duration * 60);
    clock_beforeSleep = (long)epoch_time;                      // RTC var
    clock_expectedWake = (long)epoch_time + nominal_nextSleep; // RTC var

    WakeStatus.startsleep_clock = clock_beforeSleep;
    WakeStatus.nextwake_clock = clock_expectedWake;
  }
  else // fail to obtain clock
  {
    if (debug_mode)
    {
      Serial.println("NO_REALTIME_CLOCK");
    }
    nominal_nextSleep = WakeStatus.sleep_duration * 60;
  }

  sprintf(tt, "wake_Duration: [%.2fs]; back2Sleep: [%02d:%02d:%02d]; sleepFor: [%d sec]; drift_correct: [%.1f sec]",
          millis() / 1000.0, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, nominal_nextSleep, driftRTC);

  strcat(wake_sleep_str, tt);
  Serial.println(tt);

  return nominal_nextSleep; // sec
}
void esp32Sleep::update_driftArray(float lastboot_drift)
{
  float sum_avg = 0.0;

  for (int a = drift_ArraySize - 1; a > 0; a--)
  {
    driftsArray_RTC[a] = driftsArray_RTC[a - 1];
  }

  driftsArray_RTC[0] = lastboot_drift;

  if (bootCounter > drift_ArraySize)
  {
    for (int a = 0; a < drift_ArraySize; a++)
    {
      sum_avg += driftsArray_RTC[a];
    }
    driftRTC = sum_avg / (float)drift_ArraySize;
  }
}
void esp32Sleep::new_driftUpdate(float lastboot_drift, byte cell)
{
  const float driftFactor = -0.006111;
  const float drift_tolerance = 0.25;
  const float nomin_drift = driftFactor * (WakeStatus.sleep_duration * 60.0);
  const float max_drift = nomin_drift * (1 - drift_tolerance); // bigger neg number
  const float min_drift = nomin_drift * (1 + drift_tolerance); // lesser neg number

  if (driftRTC + lastboot_drift < max_drift && driftRTC + lastboot_drift > min_drift)
  {
    driftRTC += lastboot_drift;
  }
  else if (driftRTC + lastboot_drift > min_drift)
  {
    driftRTC = min_drift;
  }
  else if (driftRTC + lastboot_drift < max_drift)
  {
    driftRTC = max_drift;
  }
  update_driftArray(driftRTC);
}
void esp32Sleep::sleepNOW(float sec2sleep)
{
  esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
bool esp32Sleep::wait_forSleep(bool wifiOK, bool nosleep) 
{
  static bool lastMessage = false;
  const int epsilonb4sleep = 300; //millis

  if (nosleep == false)
  {
    if (millis() >= WakeStatus.awake_duration * 1000)
    {
      if (wifiOK)
      {
        if (sleepduration <= 0)
        {
          if (debug_mode)
          {
            Serial.println("Error in Sleep Duration");
          }
          sleepduration = WakeStatus.sleep_duration * 60;
        }
        sleepNOW((float)sleepduration - driftRTC);
        return 0;
      }
      else
      {
        sleepNOW(WakeStatus.sleep_duration * 60.0);
        if (debug_mode)
        {
          Serial.println("NO_WIFI");
        }
        return 0;
      }
    }
    else if (millis() >= (WakeStatus.awake_duration * 1000 - epsilonb4sleep)) // this way it call ext_ fuct before sleep
    {
      if (lastMessage == false)
      {
        sleepduration = calc_nominal_sleepTime(); // seconds
        WakeStatus.drift_err = driftRTC;          // seconds

        if (_use_extfunc)
        {
          _runFunc();
        }
        lastMessage = true;
        return 0;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      return 1;
    }
  }
  else
  { // after no-sleep7
    if (millis() > no_sleep_minutes * 60 * 1000UL)
    {
      if (debug_mode)
      {
        Serial.println("no-sleep ended");
        Serial.flush();
      }
      delay(1000);
      ESP.restart();
      return 0;
    }
    else
    {
      return 1;
    }
  }
}
