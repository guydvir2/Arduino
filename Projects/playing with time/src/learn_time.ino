#include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>
#include <TimeLib.h>


void setup(){
  setTime(12,12,12,18,9,2018);

  time_t t= now();
  Serial.begin(9600);
  Serial.println(t/365/24/3600);

}

void loop(){

}
