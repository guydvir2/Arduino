#include "myIOTesp32.h"

myIOT32::myIOT32(char *ssid, char *wifi_p, char *mqtt_broker, char *mqttU, char *mqttP, char *devTopic, int port)
    : client(ssid, wifi_p, mqtt_broker, mqttU, mqttP, devTopic, port)
{
}
// void myIOT32::start(){

// }

void myIOT32::onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("mytopic/test", [](const String &payload) {
    Serial.println(payload);
  });

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  client.subscribe("mytopic/wildcardtest/#", [](const String &topic, const String &payload) {
    Serial.println(topic + ": " + payload);
  });

  // Publish a message to "mytopic/test"
  client.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  client.executeDelayed(5 * 1000, [&]() {
    client.publish("mytopic/test", "This is a message sent 5 seconds later");
  });
}

void myIOT32::looper()
{
  // client.loop();
}
