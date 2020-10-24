#include <Arduino.h>
#include <Adafruit_ADS1015.h>
#include <esp8266Sleep.h>
#include "myIOT_settings.h"

#define VoltageMeasures false

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
float ADC_bat = 0;
float ADC_solarPanel = 0;

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
#define SleepDuration 2 /* Minutes */
#define forceWake 30     /* Seconds */
esp8266Sleep espSleep;

void onWake_cb()
{
  char a[100];
  StaticJsonDocument<500> DOC;
  measureADS();
  sprintf(a, "Wake Summary: Boot [#%d], bat[%.3f v], solar[%.2f v], SleptTime [%d sec], drift [%d sec]",
          espSleep.bootCount, ADC_bat, ADC_solarPanel, espSleep.totalSleepTime, espSleep.drift);
  iot.pub_log(a);
}
void wait4OTA(){
  char a[100];
  
  espSleep.init_OTA();
  sprintf(a, "OTA: Start. Wake for[%d sec]", espSleep.sec_waitOTA);
  iot.pub_log(a);
  iot.pub_ext("OTA_TimeOUT", "", true);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  startADS();
  startIOTservices();
  espSleep.start(SleepDuration, forceWake, DEVICE_TOPIC, onWake_cb);
  espSleep.onWake_cb();
}

void loop()
{
  iot.looper();
  getEXTtopicMqtt("m");
  espSleep.wait2Sleep();
  delay(100);
}
