#include <Arduino.h>
#include <esp8266Sleep.h>
#include "general_settings.h"
#include "esp8266Sleep_param.h"
#include "myIOT_settings.h"

#include "v_measures.h"
const char *espVer = "sleepSketch_v0.6";

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
esp8266Sleep espSleep;

void onWake_cb()
{
  Serial.println("awake");
}

void onSleep_cb()
{
  char A[150];
  sprintf(A, "lastSleep: %s, nextSleep: %s, Voltages: %s", create_lastsleep_status(), create_nextsleep_status(), create_vmeasures());
  iot.pub_log(A);
  pub_debug_status();
}
void pub_debug_status()
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
  int i = retVal.length() + 1;
  char b[i];
  retVal.toCharArray(b, i);
  iot.pub_ext(b, "", true);
}
char *create_vmeasures()
{
  char *b = new char[50];

  if (vmeasure_type > 0)
  {
    get_voltage_measures();
    sprintf(b, "bat[%.2fv], solarpannel[%.1fv] ", vbat, vsolarpannel);
  }
  return b;
}
char *create_lastsleep_status()
{
  char clk[12];
  char d[5];
  espSleep.convert_epoch2clock(espSleep.totalSleepTime, 0, clk, d);
  char *b = new char[100];
  sprintf(b, "duration[%s], drift[%d sec]", clk, espSleep.drift);
  return b;
}
char *create_nextsleep_status()
{
  char clk[12];
  char d[5];
  espSleep.convert_epoch2clock(espSleep.nextsleep_duration, 0, clk, d);
  char *b = new char[100];
  sprintf(b, "duration[%s]", clk);
  return b;
}
void wait4OTA()
{
  char a[100];
  espSleep.delay_sleep(180);
  sprintf(a, "OTA: Time to OTA :[%d sec]", espSleep.sec_wait);
  iot.pub_log(a);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  startRead_parameters();
  startIOTservices();
  espSleep.start(SleepDuration, forceWake, iot.deviceTopic, onWake_cb, onSleep_cb);
  endRead_parameters();
}
void loop()
{
  iot.looper();
  espSleep.wait2Sleep();
  delay(100);
}
