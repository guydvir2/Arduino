#include <sleepyESP.h>
#include <myIOT2.h>

#define DEV_TOPIC "esp8266Sleep"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"
#define IGNORE_MQTT_BOOT_MSG true

myIOT2 iot;

void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = true;
  iot.useextTopic = false;
  iot.useDebug = true;
  iot.debug_level = 0;
  iot.useNetworkReset = true;
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
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "BOOOOO");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
    // iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help2: Commands #2 - [; m; ,x]");
    iot.pub_msg(msg);
  }
}

#define SLEEP_PERIOD 60 // minutes
#define WAKE_PERIOD 5   // seconds
#define MCU_NAME DEV_TOPIC
#define CLK_ALIGN true
sleepyESP sleepy;

void wake_cb()
{
  Serial.println("Wake up callback");
}
void sleep_cb()
{
  Serial.println("Sleep callback");
  char sleepMSG[100];

  sprintf(sleepMSG, "[Sleep]: {Boot#:%d; sleptTime_sec:%d; wakeDrift_sec:%d, nextSleep_sec:%d}", sleepy.bootCount,
          sleepy.totalSleepTime, sleepy.wake_up_drift_sec, sleepy.nextsleep_duration);
  iot.pub_msg(sleepMSG);
}

void setup()
{
  // put your setup code here, to run once:
  startIOTservices();
  sleepy.start(SLEEP_PERIOD, WAKE_PERIOD, MCU_NAME, wake_cb, sleep_cb, CLK_ALIGN);
}

void loop()
{
  // put your main code here, to run repeatedly:
  iot.looper();
  sleepy.wait2Sleep();
}
