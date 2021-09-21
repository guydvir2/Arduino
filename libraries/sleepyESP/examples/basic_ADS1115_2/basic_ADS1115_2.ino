#include <Arduino.h>
#include <sleepyESP.h>
#include <myIOT2.h>

#define DEV_TOPIC "esp8266_2_Sleep"
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

#define SLEEP_PERIOD 15 // minutes
#define WAKE_PERIOD 5   // seconds
#define MCU_NAME DEV_TOPIC
#define CLK_ALIGN true
#define READ_BAT_VOLT true

sleepyESP sleepy;
float vBAT = 0;

void wake_cb()
{
  Serial.println("Wake up callback");
}
void sleep_cb()
{
  char sleepMSG[100];
  Serial.println("Sleep callback");
  sprintf(sleepMSG, "[Sleep]: {Boot#:%d; sleptTime_sec:%d; wakeDrift_sec:%d, nextSleep_sec:%d, BAT_v:%.2f}", sleepy.bootCount,
          sleepy.totalSleepTime, sleepy.wake_up_drift_sec, sleepy.nextsleep_duration, vBAT);
  iot.pub_msg(sleepMSG);
}

#if READ_BAT_VOLT
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

bool start_ads()
{
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    return 0;
  }
  else
  {
    return 1;
  }
}
float read_ads()
{
  int16_t adc0;
  float volts;
  adc0 = ads.readADC_SingleEnded(0);
  volts = ads.computeVolts(adc0);
  return volts;
}
void getBATv(uint8_t x = 3)
{
  float sum = 0;
  uint8_t i = 0;
  if (start_ads())
  {
    while (i < x)
    {
      sum += read_ads();
      i++;
      delay(50);
    }
    vBAT = sum / ((float)x);
  }
}
#endif

void setup()
{
  startIOTservices();
  sleepy.start(SLEEP_PERIOD, WAKE_PERIOD, MCU_NAME, wake_cb, sleep_cb, CLK_ALIGN);
  getBATv(5);
}

void loop()
{
  iot.looper();
  sleepy.wait2Sleep();
}
