#include <myIOTesp32.h>
#include <myESP32sleep.h>

#define VER "ESP32_0.1v"
#define USE_VMEASURE true
#define USE_SLEEP true
// ~~~~~~~ myIOT32 ~~~~~~~~
#define DEVICE_TOPIC "ESP32"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "solarPower"
#define MQTT_TELEGRAM "myHome/Telegram"
#define USE_SERIAL true
#define USE_OTA true
#define USE_WDT false
#define USE_RESETKEEPER true
#define USE_LISTEN_TOPIC true

myIOT32 iot(DEVICE_TOPIC);

void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  iot.useResetKeeper = USE_RESETKEEPER;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.ext_mqtt_cb = ext_MQTT;
  iot.listenWakeTopic = USE_LISTEN_TOPIC;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  strcpy(iot.telegramServer, MQTT_TELEGRAM);

  iot.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Sleep ~~~~~~~~~~~
#define SLEEP_TIME 20
#define FORCE_AWAKE_TIME 20
#define NO_SLEEP_TIME 3
#define DEV_NAME DEVICE_TOPIC
bool no_sleep_flag = false;

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);
void b4sleep()
{
  Serial.println("Going to Sleep");
  Serial.flush();
  iot.getTime();
  create_wake_status("GUY_GO2BED", go2sleep.nextwake_clock, go2sleep.sleepduration, iot.epoch_time, LOW, FORCE_AWAKE_TIME);
}
void startSleep_services()
{
  go2sleep.run_func(b4sleep); // define a function to be run prior to sleep.
  go2sleep.no_sleep_minutes = NO_SLEEP_TIME;

  iot.getTime(); // generate clock and passing it to next func.
  go2sleep.startServices(&iot.timeinfo, &iot.epoch_time);
  create_wake_status("GUY_GO2BED", go2sleep.nextwake_clock, go2sleep.sleepduration, 0, HIGH, FORCE_AWAKE_TIME);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~ Power Managment ~~~~
const int measureVoltagePin = 35;
const int ADC_res = 4095;
const float Rvalue = 0.75;
const float vcc = 3.3;
float bat_volt = 0.0;

void bat_measure()
{
  byte sample = 5;
  bat_volt = 0.0;
  for (int i = 0; i < sample; i++)
  {
    bat_volt += analogRead(measureVoltagePin);
  }
  bat_volt = ((bat_volt / ((float)sample)) / ADC_res) * vcc * (1.0 / Rvalue);
}
void create_wake_status(char *cmd, long nextw, int sleept, long startSleep, bool wstat, int forcedawake)
{
  iot.DeviceStatus.wake_cmd = cmd;
  iot.DeviceStatus.nextWake_clock = nextw;
  iot.DeviceStatus.sleepduration = sleept;
  iot.DeviceStatus.wake_status = wstat;
  iot.DeviceStatus.startsleep_clock = startSleep;
  iot.DeviceStatus.forceawake = forcedawake;

  iot.createWakeJSON();
}
bool checkWake_topic()
{
  if (strcmp(iot.incmoing_wakeMSG, "maintainance") == 0)
  {
    no_sleep_flag = true;
    sprintf(iot.incmoing_wakeMSG, "");
  }
else{
  no_sleep_flag = false;

}
}

void ext_MQTT(char *incoming_msg)
{
  char msg[200];
  char msg2[20];

  if (strcmp(incoming_msg, "status") == 0)
  {
    // giveStatus(msg);
    iot.pub_msg("msg");
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver #1: Code: [%s],lib: [%s]", VER, iot.ver);
    iot.pub_msg(msg);

    sprintf(msg, "ver #2: WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], listenTopic[%d]",
            USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_LISTEN_TOPIC);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "maintainance") == 0)
  {
    sprintf(msg, "Maintainance: forced No-sleep [%d minutes] started", NO_SLEEP_TIME);
    no_sleep_flag = true;
    iot.pub_msg(msg);
    iot.pub_tele(msg);
  }
  else if (strcmp(incoming_msg, "sleep_p") == 0)
  {
    sprintf(msg, "sleep_Parameters: SleepDuration[%d minuntes], forcedWakeTime[%d sec], forced No-sleep [%d minutes]",
            SLEEP_TIME, FORCE_AWAKE_TIME, NO_SLEEP_TIME);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help") == 0)
  {
    sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg);
    sprintf(msg, "Help: Commands #2 - [sleep_p, sleepNow, maintainance(onWake Topic)]");

    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "sleepNow") == 0)
  {
    b4sleep();
    go2sleep.sleepNOW(SLEEP_TIME * 60);
  }
}

void setup()
{
  startIOT_services();
#if USE_SLEEP
  startSleep_services();
#endif

#if USE_VMEASURE
  bat_measure();
  char a[30];
  sprintf(a, "Bat measured Voltage[%.2fv]", bat_volt);
  iot.pub_msg(a);
  iot.pub_tele(a);
#endif
}

void loop()
{
  iot.looper();
#if USE_SLEEP
  iot.getTime();
  go2sleep.wait_forSleep(&iot.timeinfo, &iot.epoch_time, iot.networkOK, no_sleep_flag);
  checkWake_topic();
#endif
  delay(100);
}