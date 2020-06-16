#include <myIOTesp32.h>
#include <myESP32sleep.h>

// ~~~~~~~ myIOT32 ~~~~~~~~
#define DEVICE_TOPIC "ESP32"
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
#define FORCE_AWAKE_TIME 15
#define DEV_NAME "ESP32light"

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

void setup()
{
  startIOT_services();
  startSleep_services();
}

void loop()
{
  iot.looper();
  iot.getTime();
  go2sleep.wait_forSleep(&iot.timeinfo, &iot.epoch_time, iot.networkOK);

  sendnewNotif();
  delay(100);
}
