#include <myIOT.h>
#include"general_settings.h"
#include "powerPlug_param.h"
#include "myIOT_settings.h"
#include "mySwitch_param.h"
#include <Arduino.h>


void startGPIO()
{
  // pinMode(swPin, INPUT);
  pinMode(ledPin, OUTPUT);
  // pinMode(relayPin, OUTPUT);
  // digitalWrite(relayPin, !relayON);
  // digitalWrite(ledPin, !ledON);
}
void readSwitch()
{
  bool currentRead = digitalRead(swPin);
  delay(50);
  if (currentRead == swON && digitalRead(swPin) == swON)
  {
    relayState = !relayState;
    ledState = !ledState;
    digitalWrite(relayPin, relayState);
    digitalWrite(ledPin, ledState);
    while (digitalRead(swPin) == swON)
    {
      delay(10);
    }
  }
}

void setup()
{
  startGPIO();
  startRead_parameters();
  TOswitch_init();
  startIOTservices();
  startdTO();
  endRead_parameters();
}
void loop()
{
  iot.looper();
  TOswitch_looper();
  digitalWrite(ledPin, !digitalRead(relayPin));
  delay(100);
}
