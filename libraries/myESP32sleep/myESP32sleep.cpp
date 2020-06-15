#include "Arduino.h"
#include "myESP32sleep.h"

#define drift_ArraySize 3
#define zeroval 999.0

RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;
RTC_DATA_ATTR float driftRTC = 0;
RTC_DATA_ATTR long clock_beforeSleep = 0;
RTC_DATA_ATTR float driftsArray_RTC[drift_ArraySize];

esp32Sleep::esp32Sleep(int deepsleep, int forcedwake, char *devname)
{
  _deepsleep_time = deepsleep;
  _forcedwake_time = forcedwake;
  dev_name = devname;
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
    Serial.println("Fail to load EEPROM");
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
  }
}

// ~~~~~~~ Sleep & Drift calcs ~~~

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
  // Serial.print("Mean driftRTC: ");
  // Serial.println(driftRTC);
}
void esp32Sleep::driftUpdate(float lastboot_drift, byte cell)
{
  // if (bootCounter <= drift_ArraySize + 2)
  // {
  //   driftRTC += lastboot_drift;
  //   if (bootCounter > 2) // first 2 boots will not enter to avg_array
  //   {
  //     driftsArray_RTC[bootCounter - 3] = lastboot_drift;
  //   }
  // }
  // else
  // {
  //   float sum_avg = 0.0;
  //   for (int a = drift_ArraySize - 1; a > 0; a--)
  //   {
  //     driftsArray_RTC[a] = driftsArray_RTC[a - 1];
  //     sum_avg += driftsArray_RTC[a];
  //   }
  //   driftsArray_RTC[0] = lastboot_drift;
  //   sum_avg += lastboot_drift;
  //   driftRTC += sum_avg / (float)drift_ArraySize;

  //   if (bootCounter % 10 == 0)
  //   {
  //     saveEEPROMvalue((int)driftRTC, 0);
  //     Serial.println("DRIFT SAVED TO EEPROM");
  //   }
  // }
}
void esp32Sleep::new_driftUpdate(float lastboot_drift, byte cell)
{
  const float driftFactor = -0.006111;
  const float drift_tolerance = 0.25;
  const float nomin_drift = driftFactor * (_deepsleep_time * 60.0);
  const float max_drift = nomin_drift * (1 - drift_tolerance); // bigger neg number
  const float min_drift = nomin_drift * (1 + drift_tolerance); // lesser neg number

  if (driftRTC + lastboot_drift < max_drift && driftRTC + lastboot_drift > min_drift)
  {
    driftRTC += lastboot_drift;
    // Serial.println("drift value added");
  }
  else if (driftRTC + lastboot_drift > min_drift)
  {
    driftRTC = min_drift;
    // Serial.println("drift value corrected to min ");
  }
  else if (driftRTC + lastboot_drift < max_drift)
  {
    driftRTC = max_drift;
    // Serial.println("drift value corrected to max");
  }
  update_driftArray(driftRTC);
}

int esp32Sleep::calc_nominal_sleepTime(struct tm *timeinfo, time_t *epoch_time)
{
  int nominal_nextSleep = 0;
  char tt[100];

  if (timeinfo->tm_year >= 120)
  {
    nominal_nextSleep = _deepsleep_time * 60 - (timeinfo->tm_min * 60 + timeinfo->tm_sec) % (_deepsleep_time * 60);
    clock_beforeSleep = _epoch_time;                      // RTC var
    clock_expectedWake = _epoch_time + nominal_nextSleep; // RTC var
  }
  else // fail to obtain clock
  {
    nominal_nextSleep = _deepsleep_time * 60;
  }

  sprintf(tt, "wake_Duration: [%.2fs]; back2Sleep: [%02d:%02d:%02d]; sleepFor: [%d sec]; drift_correct: [%.1f sec]",
          millis() / 1000.0, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, nominal_nextSleep, driftRTC);
  strcat(wake_sleep_str, tt);
  Serial.println(tt);
  return nominal_nextSleep;
}
bool esp32Sleep::startServices()
{
  start_eeprom();
  return 1;
}

void esp32Sleep::sleepNOW(float sec2sleep)
{
  if (_use_extfunc)
  {
    _runFunc();
  }
  esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
void esp32Sleep::check_awake_ontime(int min_t_avoidSleep, struct tm *timeinfo, time_t *epoch_time)
{
  bootCounter++;

  sprintf(sys_presets_str, "deviceName:[%s]; SleepTime: [%d min]; Forced-aWakeTime: [%d sec]", dev_name, _deepsleep_time, _forcedwake_time);
  sprintf(wake_sleep_str, "Boot#: [%d]; Wake_Clock: [%02d:%02d:%02d]; ", bootCounter, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  Serial.println(wake_sleep_str);

  if (timeinfo->tm_year >= 120)
  {
    if (clock_beforeSleep > 0)
    {
      int wake_diff = _epoch_time - clock_expectedWake;        // not first boot
      int t_delta = wake_diff - (int)(round(millis() / 1000)); // diff between calc wake clock and current time

      char tt[100];
      sprintf(tt, "Woke_after: [%d sec]; wake_Drift: [%d sec]; ", _epoch_time - clock_beforeSleep, t_delta);
      strcat(wake_sleep_str, tt);
      new_driftUpdate(t_delta, 0);

      if (t_delta < 0 && wake_diff < 0)
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
    }
    else
    {
      saveEEPROMvalue(0, 0);
    }
  }
}
void esp32Sleep::wait_forSleep(struct tm *timeinfo, time_t *epoch_time, bool wifiOK)
{
  if (millis() >= _forcedwake_time * 1000)
  {
    if (wifiOK)
    {
      sleepNOW(calc_nominal_sleepTime(timeinfo, epoch_time) - driftRTC);
    }
    else
    {
      sleepNOW(_deepsleep_time * 60);
    }
  }
}
void esp32Sleep::run_func(cb_func cb)
{
  _runFunc = cb;
  _use_extfunc = true;
}
