#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"

LightButton Lightbut;
oper_string takeOper_string;

void LightButton::sendMSG(oper_string &str)
{
  Serial.println("HI");
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start!");
  startIOTservices();

  Lightbut.OnatBoot = false;

  Lightbut.trigType = 2;
  Lightbut.inputPressed = LOW;
  Lightbut.inputPin = D3;
  Lightbut.def_TO_minutes = 1;
  Lightbut.maxON_minutes = 100;

  // Lightbut.indicPin = D7; /* set to 255 to disable */
  Lightbut.outputPin = D6;
  Lightbut.output_ON = HIGH;
  Lightbut.outputPWM = true;
  Lightbut.defPWM = 1;
  Lightbut.max_pCount = 3;
  Lightbut.limit_PWM = 50;
  Lightbut.PWM_res = 1023;
  Lightbut.dimmablePWM = true;

  Lightbut.begin(0);
}

void loop()
{
  iot.looper();
  Lightbut.loop();
  if (Lightbut.remainWatch() != 0)
  {
    Serial.println(Lightbut.remainWatch());
  }
  delay(50);
}
