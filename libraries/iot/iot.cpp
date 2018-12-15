#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <PubSubClient.h> //MQTT
#include <Ticker.h> //WDT

// OTA libraries
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// #######################



Morse::iot(int pin)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void iot::dot()
{
}

void iot::dash()
{
}
