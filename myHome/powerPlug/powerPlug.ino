#include <myIOT.h>
#include "general_settings.h"
#include "powerPlug_param.h"
#include "myIOT_settings.h"
#include "mySwitch_param.h"
#include <Arduino.h>

void startGPIO()
{
  pinMode(swPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  turnRelay(false);
}
void turnRelay(bool state)
{
  if (state)
  {
    digitalWrite(relayPin, relayON);
    digitalWrite(ledPin, !ledON);
    relayState = relayON;
    ledState = !ledON;
    iot.pub_msg("Switch: [ON]");
  }
  else
  {
    digitalWrite(relayPin, !relayON);
    digitalWrite(ledPin, ledON);
    relayState = !relayON;
    ledState = ledON;
    iot.pub_msg("Switch: [OFF]");
  }
}
void SWlooper()
{
  bool currentRead = digitalRead(swPin);
  delay(50);
  if (currentRead == swON && digitalRead(swPin) == swON)
  {
    turnRelay(!relayState);
    while (digitalRead(swPin) == swON)
    {
      delay(10);
    }
  }
}
void autoOff(int time2off)
{
  static unsigned long onTime = 0;
  if (relayState == RelayOn && onTime == 0)
  {
    onTime = millis();
  }
  else if (relayState == RelayOn && onTime != 0)
  {
    if (millis() > onTime + time2off * 1000 * 60UL)
    {
      turnRelay(false);
      onTime = 0;
    }
  }
}
void setup()
{
  startGPIO();
  startRead_parameters();
  // TOswitch_init();
  startIOTservices();
  // startdTO();
  endRead_parameters();
}
void loop()
{
  iot.looper();
  // TOswitch_looper();
  SWlooper();
  autoOff(150);
  delay(100);
}
