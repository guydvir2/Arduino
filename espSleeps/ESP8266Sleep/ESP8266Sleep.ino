#include <Arduino.h>
#include <esp8266Sleep.h>
#include "esp8266Sleep_param.h"
#include "myIOT_settings.h"
#include "adc_measures.h"

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
const char *espVer = "sleepSketch_v0.1";
char *boardType = "esp8266_builtin_battery";
int SleepDuration = 5; /* Minutes */
int forceWake = 30;    /* Seconds */
esp8266Sleep espSleep;

void onWake_cb()
{
  char a[100];

  measureADS();
  sprintf(a, "Wake Summary: batLevel[%.2f%%], solarLevel[%.1f%%], sleepeCycle [%.2f%%], drift [%.2f%%]",
          100.0 * ADC_bat / MAX_BAT, 100.0 * ADC_solarPanel / MAX_SOLAR,
          100.0 * (float)espSleep.totalSleepTime / ((MINUTES * (float)SleepDuration) + forceWake),
          100.0 * (float)espSleep.drift / (MINUTES * (float)SleepDuration));
  iot.pub_log(a);
}
void wait4OTA()
{
  char a[100];
  espSleep.delay_sleep(180);
  sprintf(a, "OTA: Time to OTA :[%d sec]", espSleep.sec_wait);
  iot.pub_log(a);
}
void send_sleep_status()
{
  String a = create_beforeSleep_status();
  int i = a.length() + 1;
  char b[i];
  a.toCharArray(b, i);
  iot.pub_ext(b, "", true);
}
String create_beforeSleep_status()
{
  StaticJsonDocument<500> DOC;
  String retVal;

  DOC["Name"] = iot.deviceTopic;
  DOC["nominalSleep_minutes"] = SleepDuration;
  DOC["forceWake_seconds"] = forceWake;
  DOC["BootCount"] = espSleep.bootCount;
  DOC["lastSleep_seconds"] = espSleep.totalSleepTime;
  DOC["nextSleep_seconds"] = espSleep.nextsleep_duration;
  DOC["Drift_seconds"] = espSleep.drift;
  DOC["getClock"] = espSleep.clock_update_success;
  DOC["batVolt"] = ADC_bat;
  DOC["solarVolt"] = ADC_solarPanel;

  serializeJson(DOC, retVal);
  return retVal;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  startRead_parameters();
  startADS();
  startIOTservices();
  espSleep.start(SleepDuration, forceWake, iot.deviceTopic, onWake_cb, send_sleep_status);
  endRead_parameters();
}

void loop()
{
  iot.looper();
  espSleep.wait2Sleep();
  delay(100);
}
