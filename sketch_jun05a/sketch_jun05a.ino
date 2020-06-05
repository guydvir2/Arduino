#include <myIOTesp32.h>

#define DEVICE_TOPIC "ESP32"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "testBed"

myIOT32 iot;
void startIOT_services()
{
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
}
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  iot.start();
}

void loop()
{
  // put your main code here, to run repeatedly:
  //  iot.startMQTT();
  iot.looper();
  delay(200);
}
