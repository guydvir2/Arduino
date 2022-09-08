#include <Arduino.h>
#include <ArduinoJson.h>


char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
DynamicJsonDocument doc(1024);
DynamicJsonDocument doc2(1024);


void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");


  deserializeJson(doc, json);
  for(uint8_t i=0;i<13;i++){
  Serial.println(doc["data"][i].containsKey());
  }
}

void loop()
{
}
