RTC_DATA_ATTR long clock_beforeSleep = 0;
RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;

class esp32Sleep
{
#define DEV_NAME "ESP32lite"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
#define uS_TO_S_FACTOR 1000000ULL    /* Conversion micro seconds to seconds */
#define TIME_TO_SLEEP DEEPSLEEP_TIME /* minutes in deep sleep */
#define TIME_AWAKE FORCED_WAKE_TIME  /* Seconds until deep sleep */

private:
  char sleepstr[250];
  const int gmtOffset_sec = 2 * 3600;
  const int daylightOffset_sec = 0; //3600;
  struct tm timeinfo;
  time_t epoch_time;
  char clock_char[20];
  char date_char[20];

  void startNTP()
  {
    const char *ntpServer = "pool.ntp.org";
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  }

  bool getTime()
  {
    byte a;
    while (a <= 2)
    {
      if (getLocalTime(&timeinfo))
      {
        time(&epoch_time);
        return 1;
      }
      delay(50);
      a++;
    }
    return 0;
  }

public:
#include "time.h"

  void esp32()
  {
  }
  void sleepNOW(int sec2sleep = 2700)
  {
    char tmsg[30];

    sprintf(tmsg, "Going to DeepSleep for [%d] sec", sec2sleep);
    Serial.println(tmsg);
    Serial.println("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    // mqtt_pubmsg(tmsg);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }

  void check_awake_ontime(int min_t_avoidSleep = 10)
  {
    delay(2500);
    getTime();
    bootCounter++;

    // Serial.print("WAKE CLOCK: ");
    // printClock();
    // Serial.print("WAKE epoch:");
    // Serial.println(epoch_time);
    // Serial.print("last epoch:");
    // Serial.println(clock_beforeSleep);
    sprintf(sleepstr, "deviceName:[%s]; nominalSleep: [%d min]; ForcedWakeTime: [%d sec];  Boot#: [%d]; WakeupClock: [%02d:%02d:%02d];",
            DEV_NAME, DEEPSLEEP_TIME, FORCED_WAKE_TIME, bootCounter, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    if (timeinfo.tm_year >= 120) // year 2020
    {
      if (clock_beforeSleep > 0)
      {                                                                         // not first boot
        int t_delta = epoch_time - clock_expectedWake - (int)(millis() / 1000); // diff between calc wake clock and current time

        char tt[100];
        sprintf(tt, "lastSleep: [%d sec]; drift_lastSleep: [%d sec]; ", clock_expectedWake - clock_beforeSleep, t_delta);
        Serial.println(tt);
        strcat(sleepstr, tt);

        bool up = updateDrift_EEPROM(t_delta, 0);
        {
          sprintf(tt, "driftUpdate: [%s]; ", up ? "YES" : "NO");
          Serial.println(tt);
          strcat(sleepstr, tt);

          // Serial.print("drift value updated: ");
          // Serial.print(t_delta);
          // Serial.println(" sec");
        }

        if (t_delta >= 0)
        {
          Serial.println("OK - WOKE UP after due time: ");
        }
        else
        {
          Serial.println("FAIL- woke up before time: ");
          int tempSleep = epoch_time - clock_expectedWake;
          if (abs(tempSleep) < min_t_avoidSleep)
          {
            sprintf(tt, "syncPause: [%d sec]; ", abs(tempSleep));
            strcat(sleepstr, tt);
            // Serial.print("pausing ");
            // Serial.print(tempSleep);
            // Serial.println(" sec");

            delay(1000 * abs(tempSleep));
          }
          else
          {
            // Serial.println("going to temp sleep");
            sleepNOW(-1 * tempSleep);
          }
        }
      }
      else
      {
        saveEEPROMvalue(0, 0);
      }
    }
    else
    {
      Serial.println("BAD NTP");
    }
  }
};

void setup()
{
  // put your setup code here, to run once:
}

void loop()
{
  // put your main code here, to run repeatedly:
}
