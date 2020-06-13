#include <myIOTesp32.h>

#define DEVICE_TOPIC "ESP32"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "testBed"
#define USE_SERIAL true
#define USE_OTA false

myIOT32 iot(DEVICE_TOPIC, SSID_ID, PASS_WIFI, MQTT_SERVER1);
void ext_MQTT()
{
}
void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  iot.useResetKeeper = true;
  iot.useWDT = false;
  iot.useOTA = USE_OTA;
  // iot.ext_mqtt_cb = &ext_MQTT;
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
  delay(500);
}
