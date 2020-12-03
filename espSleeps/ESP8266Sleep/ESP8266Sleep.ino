#include <Arduino.h>
#include <esp8266Sleep.h>
#include "general_settings.h"
#include "esp8266Sleep_param.h"
#include "myIOT_settings.h"

#include "v_measures.h"

const char *espVer = "sleepSketch_v0.2";

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
esp8266Sleep espSleep;

void onWake_cb()
{
  char a[100];
  char b[50];

  bool got_v = get_voltage_measures();
  if (got_v)
  {
    sprintf(b, "bat[%.2fv], solarpannel[%.1fv] ", vbat, vsolarpannel);
  }
  sprintf(a, "Wake Summary: %ssleepeCycle [%.2f%%], drift [%.2f%%]", got_v ? b : "",
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
  DOC["batVolt"] = vbat;
  DOC["solarVolt"] = vsolarpannel;

  serializeJson(DOC, retVal);
  return retVal;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  startRead_parameters();
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
