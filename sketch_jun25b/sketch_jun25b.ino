#include <myIOT.h>
#define ADD_MQTT_FUNC addiotnalMQTT
#define DEVICE_TOPIC "HomePi/Dvir/Lights/sono"

myIOT iot(DEVICE_TOPIC);

void addiotnalMQTT(char *incoming_msg) {}
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  iot.looper();

}
