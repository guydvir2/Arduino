#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"

LightButton<1> Lightbut;
oper_string takeOper_string;

template<uint8_t N>
void LightButton<N>::sendMSG(oper_string &str)
{
  Serial.println("HI");
}

void setup()
{
  startIOTservices();

  Lightbut.define_button(0, 0, D3, LOW, 1, 10);
  Lightbut.define_light(0, D6, HIGH, true, true);


  // Lightbut.begin(0);
}

void loop()
{
  iot.looper();
  Lightbut.loop();
  // if (Lightbut.remainClock() != 0)
  // {
  //   Serial.println(Lightbut.remainClock());
  // }
  //   delay(50);
}
