#include <Arduino.h>

#define DEBUG_MODE false
#define MAX_PAYLOAD_SIZE 75

#if defined(ARDUINO_ARCH_ESP8266)
#include "myIOT_settings.h"
#define isESP8266 true
#define ROLE 0 /* 0:Reciever 1: Sender */
#else
#define isESP8266 false
#define ROLE 1
#endif

#include "RF24_DEFS.h"

void (*resetFunc)(void) = 0;

void setup()
{
#if isESP8266
  startIOTservices();
#else
  Serial.begin(115200);
#endif

  RF24_init();
}
void loop()
{
#if isESP8266
  iot.looper();
#endif
  RF24_Rx_looper();
  radio.wellness_Watchdog();
}
