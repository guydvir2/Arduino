#include <myIOTesp32.h>
#include <myESP32sleep.h>

// ~~~~~~~ myIOT32 ~~~~~~~~
#define DEVICE_TOPIC "ESP32_2"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "testBed"
#define USE_SERIAL true
#define USE_OTA true

myIOT32 iot(DEVICE_TOPIC);

void ext_MQTT(char *incoming)
{
  return;
}
void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  iot.useResetKeeper = true;
  iot.useWDT = false;
  iot.useOTA = USE_OTA;
  iot.useTelegram = false;
  iot.ext_mqtt_cb = ext_MQTT;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  iot.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Sleep ~~~~~~~~~~~
#define SLEEP_TIME 2
#define FORCE_AWAKE_TIME 45
#define DEV_NAME "NODE-ESP32"

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);
void b4sleep()
{
  Serial.println("Going to Sleep");
  Serial.flush();
}
void startSleep_services()
{
  go2sleep.run_func(b4sleep); // define a function to be run prior to sleep.
  iot.getTime();              // generate clock and passing it to next func.
  go2sleep.startServices(&iot.timeinfo, &iot.epoch_time);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~ Power Managment ~~~~
const int measureVoltagePin = 35;
float bat_volt = 0.0;

void bat_measure()
{
  byte sample = 5;
  bat_volt = 0.0;
  for (int i = 0; i < sample; i++)
  {
    bat_volt += analogRead(measureVoltagePin);
  }
  bat_volt = (bat_volt / ((float)sample)) * (15.0 / 4095.0);
  Serial.println(bat_volt);
}
void sendnewNotif()
{
  static bool notified = false;
  iot.getTime();
  int divider = 2;
  if (iot.timeinfo.tm_min % divider == 0 && notified == false)
  {
    char timeStamp[25];
    iot.getTimeStamp(timeStamp);
    iot.mqttClient.publish("myHome/Telegram", timeStamp);
    notified = true;
  }
  else if (iot.timeinfo.tm_min % divider != 0 && notified == true)
  {
    notified = false;
  }
}
void create_wake_status(char *cmd, long nextw, int sleept, bool wstat)
{
  iot.DeviceStatus.wake_cmd = cmd;
  iot.DeviceStatus.nextWake = nextw;
  iot.DeviceStatus.sleeptime = sleept;
  iot.DeviceStatus.wake_status = wstat;
}
void setup()
{
  startIOT_services();
  startSleep_services();
  bat_measure();
  char a[30];
  sprintf(a, "Bat measured Voltage[%.2fv]", bat_volt);
  iot.pub_msg(a);
}

void loop()
{
  iot.looper();
  iot.getTime();
  go2sleep.wait_forSleep(&iot.timeinfo, &iot.epoch_time, iot.networkOK);

  sendnewNotif();
  delay(100);
}
