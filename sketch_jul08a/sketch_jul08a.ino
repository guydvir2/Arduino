#include <Arduino.h>
#include <WiFiUdp.h>      // OTA
#include <ArduinoOTA.h>   // OTA
#include <Ticker.h>       //WDT
#include <PubSubClient.h> // MQTT
#include <ArduinoJson.h>
#include <Chrono.h>

#if defined(ESP8266)
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // OTA libraries
#include <TZ.h>
#define LITFS LittleFS

#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h> // OTA libraries
#include <ESP32Ping.h>
#include "LITTLEFS.h"
#define LITFS LITTLEFS
#define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")

#endif

#if defined(ESP8266)
#include <ESP8266Ping.h>
#endif

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStart");

  // for (uint8_t i = 0; i < 4; i++)
  // {
  //   pinMode(output_pins[i], OUTPUT);
  //   digitalWrite(output_pins[i], LOW);
  //   delay(100);
  //   digitalWrite(output_pins[i], HIGH);
  //   delay(1000);
  //   digitalWrite(output_pins[i], LOW);
  //   delay(1000);
  // }

  // for (uint8_t i = 0; i < 4; i++)
  // {
  //   pinMode(input_pins[i], INPUT_PULLUP);
  // }
}

void loop()
{
  //    for (uint8_t i = 0; i < 4; i++)
  // {
  // char a[20];
  // sprintf(a, "%d;%d;%d;%d", digitalRead(input_pins[0]), digitalRead(input_pins[1]), digitalRead(input_pins[2]), digitalRead(input_pins[3]));
  // Serial.println(a);
  // delay(500);
  // }
}
