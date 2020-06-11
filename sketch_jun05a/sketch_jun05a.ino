#include <myIOTesp32.h>

#define DEVICE_TOPIC "ESP32"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "testBed"
#define USE_SERIAL true
#define USE_OTA false

myIOT32 iot(DEVICE_TOPIC, "iPhone", "guyd5161", MQTT_SERVER3);
void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  // iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  iot.start();
}
void setup()
{
  // Serial.begin(9600);
  startIOT_services();
  Serial.println("\nStart!");
  iot.pub_msg("Gmorning");
}

void loop()
{
  // put your main code here, to run repeatedly:
  iot.looper();
  delay(200);
}
