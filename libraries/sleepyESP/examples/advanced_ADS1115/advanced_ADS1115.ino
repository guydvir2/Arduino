#include <Arduino.h>
#include <sleepyESP.h>
#include <myIOT2.h>

#define DEV_TOPIC "esp8266_Bat"
#define GROUP_TOPIC "sleepy"
#define PREFIX_TOPIC "myHome"
#define USE_EXT_TOPIC true
#define MQTT_EXT_TOPIC PREFIX_TOPIC "/" GROUP_TOPIC "/" DEV_TOPIC "/" \
                                    "onWake"
#define IGNORE_MQTT_BOOT_MSG true

#define SLEEP_PERIOD 60 // minutes
#define WAKE_PERIOD 15  // seconds
#define MCU_NAME DEV_TOPIC
#define CLK_ALIGN true
#define READ_BAT_VOLT true

myIOT2 iot;
sleepyESP sleepy;

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
float getBATv(uint8_t x = 3)
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
    return sum / ((float)x);
  }
  else
  {
    return -99.9;
  }
}
#endif

void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = true;
  iot.useextTopic = USE_EXT_TOPIC;
  iot.useDebug = true;
  iot.debug_level = 1;
  iot.useNetworkReset = false;
  iot.noNetwork_reset = 10;
  iot.useBootClockLog = true;
  iot.ignore_boot_msg = IGNORE_MQTT_BOOT_MSG; // <---- This is for us only //
  iot.extTopic[0] = MQTT_EXT_TOPIC;           // <---- This is for us only //

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
    sprintf(msg, "I'm awake for %.2f[sec]",(float)(millis()/1000.0));
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
void start_maintainance()
{
  char m[100];
  const uint8_t POSTPONE_SLEEP_SEC = 120;
  sleepy.delay_sleep(POSTPONE_SLEEP_SEC);
  Serial.println("maintainance");
  sprintf(m, "Maintainance: Sleep delay %d[sec]", POSTPONE_SLEEP_SEC);
  iot.pub_log(m);
  iot.pub_ext("maintainance_done", DEV_TOPIC, true);
}
void checkWake_topic()
{
  if (strcmp(iot.extTopic_msg.msg, "m") == 0) /* retained msg "m" will start maintenance mode*/
  {
    start_maintainance();
    iot.clear_ExtTopicbuff();
  }
}
void wake_cb()
{
  Serial.println("Wake up callback");
}
void sleep_cb()
{
  char sleepMSG[100];
  float vBAT = 0;

#if READ_BAT_VOLT
  vBAT = getBATv(5);
#endif

  Serial.println("Sleep callback");
  sprintf(sleepMSG, "[Sleep]: {Boot#:%d; sleptTime_sec:%d; wakeDrift_sec:%d, nextSleep_sec:%d, BAT_v:%.2f}", sleepy.bootCount,
          sleepy.totalSleepTime, sleepy.wake_up_drift_sec, sleepy.nextsleep_duration, vBAT);
  iot.pub_msg(sleepMSG);
}



void setup()
{
#if ESP32
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
#endif
  startIOTservices();
  sleepy.start(SLEEP_PERIOD, WAKE_PERIOD, MCU_NAME, wake_cb, sleep_cb, CLK_ALIGN);
}
void loop()
{
  iot.looper();
  checkWake_topic();
  sleepy.wait2Sleep();
}
