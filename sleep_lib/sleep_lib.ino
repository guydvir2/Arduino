#include "EEPROM.h"
#include "time.h"
#include "WiFi.h"

#define DEV_NAME "ESP32lite"

RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;
RTC_DATA_ATTR int driftRTC = 0;
RTC_DATA_ATTR long clock_beforeSleep = 0;

class esp32Sleep
{
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define EEPROM_SIZE 16

private:
  char sleepstr[250];
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
    long beginwifi = millis();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && millis() - beginwifi < 30000)
    {
      delay(200);
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

  bool driftUpdate(int drift_value, byte cell = 0, byte update_freq = 10)
  {
    if (abs(drift_value) >= 2)
    {
      driftRTC += drift_value;
    }

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

  int calc_nominal_sleepTime()
  {
    int nominal_nextSleep = 0;
    char tt[100];

    if (getTime())
    {
      nominal_nextSleep = deepsleep_time * 60 - (timeinfo.tm_min * 60 + timeinfo.tm_sec) % (deepsleep_time * 60);
      clock_beforeSleep = epoch_time;                      // RTC var
      clock_expectedWake = epoch_time + nominal_nextSleep; // RTC var
      Serial.print("expected wake CLOCK: ");
      Serial.println(clock_expectedWake);
    }
    else // fail to obtain clock
    {
      nominal_nextSleep = deepsleep_time * 60;
    }

    sprintf(tt, "wakeDuration: [%.2fs]; startSleep: [%02d:%02d:%02d]; sleepFor: [%d sec]; drift: [%d sec]",
            (float)millis() / 1000.0, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, nominal_nextSleep, driftRTC);
    strcat(sleepstr, tt);
    return nominal_nextSleep;
  }

public:
  int deepsleep_time = 0;
  int forcedwake_time = 0;
  bool network_status = false;
  bool start_wifi = true;
  char *dev_name = "myESP32_devname";

  esp32Sleep(int deepsleep = 30, int forcedwake = 15, char *devname = "dev")
  {
    deepsleep_time = deepsleep;
    forcedwake_time = forcedwake;
    dev_name = devname;
  }
  bool startServices()
  {
    start_eeprom();
    if (start_wifi)
    {
      network_status = startWifi();
      if (network_status)
      {

        check_awake_ontime();
      }
      return 1;
    }
    else
    {
      sprintf(sleepstr, "Fail to obtain WiFi");
      return 0;
    }
  }

  void printClock()
  {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    // Serial.print(timeinfo.tm_hour);
    // Serial.print(":");
    // Serial.print(timeinfo.tm_min);
    // Serial.print(":");
    // Serial.print(timeinfo.tm_sec);
    // Serial.println("");
  }

  void sleepNOW(int sec2sleep = 2700)
  {
    char tmsg[30];
    Serial.println(sleepstr);
    sprintf(tmsg, "Going to DeepSleep for [%d] sec", sec2sleep);
    Serial.println(tmsg);
    Serial.println("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    Serial.flush();
    esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }

  void check_awake_ontime(int min_t_avoidSleep = 10)
  {
    delay(3000);
    getTime();
    printUpdatedClock("Wake");
    // Serial.print("WAKE CLOCK: ");
    // printClock();
    // Serial.print("WAKE epoch:");
    // Serial.println(epoch_time);
    Serial.print("last epoch:");
    Serial.println(clock_beforeSleep);
    bootCounter++;

    sprintf(sleepstr, "deviceName:[%s]; nominalSleep: [%d min]; ForcedWakeTime: [%d sec];  Boot#: [%d]; WakeupClock: [%02d:%02d:%02d];",
            dev_name, deepsleep_time, forcedwake_time, bootCounter, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    if (timeinfo.tm_year >= 120)
    {
      if (clock_beforeSleep > 0)
      {   
        int wake_diff = epoch_time - clock_expectedWake;                                                                       // not first boot
        int t_delta = wake_diff - (int)(round(millis() / 1000)); // diff between calc wake clock and current time

        // int t_delta = (int)(round(millis() / 1000)) - (epoch_time - clock_expectedWake); // diff between calc wake clock and current time
        Serial.print("T_DELTA: ");
        Serial.println(t_delta);

        char tt[100];
        sprintf(tt, "lastSleep: [%d sec]; drift_lastSleep: [%d sec]; ", clock_expectedWake - clock_beforeSleep, t_delta);
        strcat(sleepstr, tt);

        bool up = driftUpdate(t_delta, 0, 5);
        sprintf(tt, "driftUpdate: [%s]; ", up ? "YES" : "NO");
        strcat(sleepstr, tt);

        if (t_delta < 0)
        {
          if (wake_diff < 0 && abs(wake_diff) <= min_t_avoidSleep)
          {
            sprintf(tt, "syncPause: [%d sec]; ", abs(wake_diff));
            strcat(sleepstr, tt);
            Serial.print("temp_sleep: ");
            Serial.println(abs(wake_diff));
            delay(1000 * abs(wake_diff));
          }
          else if (wake_diff < 0 && abs(wake_diff) > min_t_avoidSleep)
          {
            sleepNOW(abs(wake_diff));
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
  void wait_forSleep()
  {
    if (millis() >= forcedwake_time * 1000)
    {
      if (network_status)
      {
        printUpdatedClock("Sleep Summery");
        sleepNOW(calc_nominal_sleepTime() - driftRTC);
      }
      else
      {
        Serial.println(sleepstr);
        sleepNOW(deepsleep_time * 60);
      }
    }
  }
  void printUpdatedClock(char *hdr = "")
  {
    Serial.print("\n~~~~~~");
    Serial.print(hdr);
    Serial.print("~~~~~~\n");
    getTime();
    printClock();
    Serial.print("epoch:");
    Serial.println(epoch_time);
    Serial.print("~~~~~~");
    Serial.print("end");
    Serial.print("~~~~~~\n\n");
  }
};

esp32Sleep go2sleep(60, 15, DEV_NAME);

void setup()
{
  Serial.begin(9600);
  Serial.println("\n~~~~~~ Boot ~~~~~~");
  go2sleep.start_wifi = true;
  go2sleep.startServices();
}

void loop()
{
  go2sleep.wait_forSleep();
}
