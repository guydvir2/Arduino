#include <sleepyESP.h>
#include <myIOT2.h>

#define DEV_TOPIC "espSleep"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"

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
  iot.ignore_boot_msg = true;
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

#define SLEEP_PERIOD 2 // minutes
#define WAKE_PERIOD 5  // seconds
sleepyESP sleepy;

void wake_cb()
{
  Serial.println("WAKE");
}
void sleep_cb()
{
  Serial.println("going to sleep");
}

void setup()
{
  // put your setup code here, to run once:
  startIOTservices();
  sleepy.start(SLEEP_PERIOD, WAKE_PERIOD, "testDrive", wake_cb, sleep_cb, true);
}

void loop()
{
  // put your main code here, to run repeatedly:
  iot.looper();
  sleepy.wait2Sleep();
}
