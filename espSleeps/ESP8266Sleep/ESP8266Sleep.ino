#include <Arduino.h>
#include <esp8266Sleep.h>
#include "general_settings.h"
#include "esp8266Sleep_param.h"
#include "myIOT_settings.h"

#include "v_measures.h"

const char *espVer = "sleepSketch_v0.5";

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
esp8266Sleep espSleep;

void onWake_cb()
{
  char a[100];
  char b[50];

  if (vmeasure_type > 0)
  {
    get_voltage_measures();
    sprintf(b, "bat[%.2fv], solarpannel[%.1fv] ", vbat, vsolarpannel);
  }
  sprintf(a, "Wake Summary: %ssleepeCycle [%.2f%%], drift [%.2f%%]", vmeasure_type > 0 ? b : "",
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
  DOC["nominalSleep[min]"] = SleepDuration;
  DOC["forceWake[sec]"] = forceWake;
  DOC["BootCount"] = espSleep.bootCount;
  DOC["lastSleep[sec]"] = espSleep.totalSleepTime;
  DOC["nextSleep[sec]"] = espSleep.nextsleep_duration;
  DOC["Drift[sec]"] = espSleep.drift;
  DOC["getClock"] = espSleep.clock_update_success;
  DOC["bat[v]"] = vbat;
  DOC["solar[v]"] = vsolarpannel;

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
