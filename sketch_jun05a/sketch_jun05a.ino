#include <myIOTesp32.h>

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
void sendnewNotif()
{
  static bool notified = false;
  iot.getTime();
  int divider = 2;
  if (iot.timeinfo.tm_min % divider == 0 && notified == false)
  {
    char timeStamp[25];
    iot.getTimeStamp(timeStamp);
    iot.mqttClient.publish("myHome/Telegram_out", timeStamp);
    notified = true;
    Serial.println(timeStamp);
  }
  else if (iot.timeinfo.tm_min % divider != 0 && notified == true)
  {
    notified = false;
  }
}

void setup()
{
  startIOT_services();
}

void loop()
{
  iot.looper();
  sendnewNotif();
  delay(100);
}
