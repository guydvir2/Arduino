#include "EEPROM.h"
#include "time.h"
#include "WiFi.h"

RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;
RTC_DATA_ATTR int driftRTC = 0;
RTC_DATA_ATTR long clock_beforeSleep = 0;

class esp32Sleep
{
#define EEPROM_SIZE 16
#define DEV_NAME "ESP32lite"
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */

private:
  char sleepstr[250];
  // RTC_DATA_ATTR long clock_expectedWake = 0;

  struct tm timeinfo;
  time_t epoch_time;

  const char *ssid = "Xiaomi_D6C8";
  const char *password = "guyd5161";

  int getEEPROMvalue(byte i = 0)
  {
    int eeprom_drift = EEPROM.read(i) * pow(-1, EEPROM.read(i + 1));
    return eeprom_drift;
  }
  void saveEEPROMvalue(int val, byte i = 0)
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
  void start_eeprom(byte i = 0)
  {
    if (!EEPROM.begin(EEPROM_SIZE))
    {
      Serial.println("Fail to load EEPROM");
    }
  }
  void startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 0, const char *ntpServer = "pool.ntp.org")
  {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  }
  bool startWifi()
  {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && millis() < 30000)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println(WiFi.localIP());
      startNTP();
      return 1;
    }
    else
    {
      Serial.println("Failed connect to wifi");
      return 0;
    }
  }
  bool getTime()
  {
    byte a;
    while (a < 3)
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
  int deepsleep_time = 0;
  int forcedwake_time = 0;
  bool network_status = false;

  esp32Sleep(int deepsleep = 30, int forcedwake = 15)
  {
    deepsleep_time = deepsleep;
    forcedwake_time = forcedwake;
  }
  bool start_all()
  {
    start_eeprom();
    network_status = startWifi();
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

  bool driftUpdate(int drift_value, byte cell = 0, byte update_freq = 10)
  {
    driftRTC += drift_value;

    if (bootCounter <= 2 || bootCounter % update_freq == 0)
    {
      if (abs(driftRTC - getEEPROMvalue(cell)) > 2)
      {
        saveEEPROMvalue(driftRTC, cell);
        return 1;
      }
    }
    else
    {
      return 0;
    }
  }

  void check_awake_ontime(int min_t_avoidSleep = 10)
  {
    delay(2500);
    getTime();
    bootCounter++;

    // Serial.print("WAKE CLOCK: ");
    // // printClock();
    // Serial.print("WAKE epoch:");
    // Serial.println(epoch_time);
    // Serial.print("last epoch:");
    // Serial.println(clock_beforeSleep);

    sprintf(sleepstr, "deviceName:[%s]; nominalSleep: [%d min]; ForcedWakeTime: [%d sec];  Boot#: [%d]; WakeupClock: [%02d:%02d:%02d];",
            DEV_NAME, deepsleep_time, forcedwake_time, bootCounter, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    if (timeinfo.tm_year != 0)
    {
      if (clock_beforeSleep > 0)
      {                                                                         // not first boot
        int t_delta = epoch_time - clock_expectedWake - (int)(millis() / 1000); // diff between calc wake clock and current time

        char tt[100];
        sprintf(tt, "lastSleep: [%d sec]; drift_lastSleep: [%d sec]; ", clock_expectedWake - clock_beforeSleep, t_delta);
        Serial.println(tt);
        strcat(sleepstr, tt);

        bool up = driftUpdate(t_delta, 0, 5);
        sprintf(tt, "driftUpdate: [%s]; ", up ? "YES" : "NO");
        Serial.println(tt);
        strcat(sleepstr, tt);

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
            sleepNOW(abs(tempSleep));
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

  int calc_nominal_sleepTime()
  {
    int nominal_nextSleep = 0;
    char tt[100];

    if (getTime())
    {
      nominal_nextSleep = TIME_TO_SLEEP * 60 - (timeinfo.tm_min * 60 + timeinfo.tm_sec) % (TIME_TO_SLEEP * 60);
      clock_beforeSleep = epoch_time;                      // RTC var
      clock_expectedWake = epoch_time + nominal_nextSleep; // RTC var
    }
    else // fail to obtain clock
    {
      nominal_nextSleep = TIME_TO_SLEEP * 60;
    }
    
    sprintf(tt, "wakeDuration: [%.2fs]; startSleep: [%02d:%02d:%02d]; sleepFor: [%d sec]; drift: [%d sec]",
            (float)millis() / 1000.0, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, nominal_nextSleep, driftRTC);
    strcat(sleepstr, tt);
    return nominal_nextSleep;
  }
};

esp32Sleep go2sleep(30, 15);

void setup()
{
  Serial.begin(9600);
  go2sleep.start_all();
  go2sleep.check_awake_ontime();
  go2sleep.sleepNOW(20);
  // put your setup code here, to run once:
}

void loop()
{
  // Serial.println("LOOP");
  // put your main code here, to run repeatedly:
}
