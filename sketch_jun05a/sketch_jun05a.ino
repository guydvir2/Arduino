#include <myIOTesp32.h>

#define DEVICE_TOPIC "ESP32"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "testBed"
#define USE_SERIAL true
#define USE_OTA false

myIOT32 iot(DEVICE_TOPIC);
void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  iot.useResetKeeper = true;
  // iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  iot.start();
}
void setup()
{
  startIOT_services();
}

void loop()
{
  iot.looper();
  delay(200);
}
