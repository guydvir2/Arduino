#include <myIOT2.h>
#include <sleepyESP.h>

#define DEV_TOPIC "esp32"
#define GROUP_TOPIC "sleepy"
#define PREFIX_TOPIC "myHome"
#define IGNORE_MQTT_BOOT_MSG true
#define MCU_NAME DEV_TOPIC

#define WAKE_PERIOD 20 // seconds
#define SLEEP_PERIOD 1 // minutes
#define CLK_ALIGN true
#define READ_BAT_VOLT true
#define READ_CELL_VOLT true

myIOT2 iot;
sleepyESP sleepy;

#if READ_BAT_VOLT
#include "voltage_measure.h"
#endif

void startIOTservices()
{
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useSerial = true;
  iot.useResetKeeper = true;
  iot.useextTopic = false;
  iot.useDebug = true;
  iot.debug_level = 1;
  iot.useNetworkReset = false;
  iot.noNetwork_reset = 10;
  iot.useBootClockLog = true;
  iot.ignore_boot_msg = IGNORE_MQTT_BOOT_MSG; // <---- This is for us only //

  strcpy(iot.deviceTopic, DEV_TOPIC);
  strcpy(iot.prefixTopic, PREFIX_TOPIC);
  strcpy(iot.addGroupTopic, GROUP_TOPIC);
  iot.start_services(addiotnalMQTT);
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "I'm awake for %.2f[sec]", (float)(millis() / 1000.0));
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "ver #2: sleepyESP [%s], boardType[%s]", sleepy.VER, isESP32 ? "ESP32" : "ESP8266");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help2: Commands #2 - [m {call maintainance - freeze sleep to OTA}, sleep_now]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "m") == 0) /* retained msg "m" will start maintenance mode*/
  {
    start_maintainance();
  }
  else if (strcmp(incoming_msg, "sleep_now") == 0) // overide wait
  {
    sleepy.startSleepNOW = true;
  }
}

void start_maintainance()
{
  char m[100];
  const uint8_t POSTPONE_SLEEP_SEC = 180;

  sleepy.delay_sleep(POSTPONE_SLEEP_SEC);
  Serial.println("maintainance");
  sprintf(m, "Maintainance: Sleep delay %d[sec]", POSTPONE_SLEEP_SEC);
  iot.pub_log(m);

  // Clear retained topic
  sprintf(m, "%s/%s/%s", PREFIX_TOPIC, GROUP_TOPIC, DEV_TOPIC);
  iot.pub_noTopic("", m, true);
  //

  wake_cb();
}

void wake_cb()
{
  Serial.println("Wake up callback");
}
void sleep_cb()
{
  char sleepMSG[100];
  float vBAT = 0;
  float vCell = 0;

#if READ_BAT_VOLT
  vBAT = get_voltage(5, 0);
  vCell = get_voltage(5, 1);
#endif
  Serial.println("Sleep callback");
  sprintf(sleepMSG, "[Sleep]: {Boot#:%d; sleptTime_sec:%d; wakeDrift_sec:%d, nextSleep_sec:%d, BAT_v:%.2f, Cell_v:%.2f}",
          sleepy.bootCount, sleepy.totalSleepTime, sleepy.wake_up_drift_sec, sleepy.nextsleep_duration, vBAT, vCell);

  Serial.println(sleepMSG);
  iot.pub_msg(sleepMSG);
  delay(500);
}

void setup()
{
#if defined(ESP32)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
#endif
  startIOTservices();
  sleepy.start(SLEEP_PERIOD, WAKE_PERIOD, wake_cb, sleep_cb, CLK_ALIGN);
}
void loop()
{
  iot.looper();
  sleepy.wait2Sleep();
  sleepy.startSleepNOW = false;
}
