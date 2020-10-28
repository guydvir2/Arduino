#include <Arduino.h>
#include <Adafruit_ADS1015.h>
#include <esp8266Sleep.h>
#include "myIOT_settings.h"

#define VoltageMeasures false

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
float ADC_bat = 0;
float ADC_solarPanel = 0;
const float MAX_BAT = 4.1;
const float MAX_SOLAR = 6.0;

#if VoltageMeasures
Adafruit_ADS1115 ads;

const float ADC_convFactor = 0.1875;
const float solarVoltageDiv = 0.66;
#endif

void startADS()
{
#if VoltageMeasures
  ads.begin();
#endif
}
void measureADS()
{
#if VoltageMeasures
  ADC_bat = ads.readADC_SingleEnded(0) * ADC_convFactor * 0.001;
  ADC_solarPanel = ads.readADC_SingleEnded(1) * ADC_convFactor * 0.001 / solarVoltageDiv;
#endif
}

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
#define SleepDuration 60 /* Minutes */
#define forceWake 30    /* Seconds */
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
  sprintf(a, "OTA: Start. Wake for[%d sec]", espSleep.sec_wait);
  iot.pub_log(a);
  iot.pub_ext("OTA_TimeOUT", "", true);
}
void send_sleep_status()
{
  String a = create_beforeSleep_status();
  int i = a.length() + 1;
  char b[i];
  a.toCharArray(b, i);

  iot.pub_ext(b);
}
String create_beforeSleep_status()
{
  StaticJsonDocument<500> DOC;
  String retVal;

  DOC["Name"] = DEVICE_TOPIC;
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
  startADS();
  startIOTservices();
  espSleep.start(SleepDuration, forceWake, DEVICE_TOPIC, onWake_cb,send_sleep_status);
}

void loop()
{
  iot.looper();
  espSleep.wait2Sleep();
  delay(100);
}
