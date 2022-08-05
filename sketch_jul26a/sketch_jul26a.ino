#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"

LightButton<2> Lightbut;

void print_OPERstring(oper_string &str, uint8_t i)
{
  Serial.print(" +++++++++ OPER_STRING START #");
  Serial.print(i);
  Serial.println(" +++++++++");

  Serial.print("state:\t\t\t");
  Serial.println(str.state);

  Serial.print("reason:\t\t\t");
  Serial.println(str.reason);

  Serial.print("step:\t\t\t");
  Serial.println(str.step);

  Serial.print("ontime:\t\t\t");
  Serial.println(str.ontime);

  Serial.print("offtime:\t\t");
  Serial.println(str.offtime);
  Serial.println("######### OPER_STRING END ###########");
}

template <uint8_t N>
void LightButton<N>::sendMSG(oper_string &str, uint8_t i)
{
  print_OPERstring(str, i);
}

void setup()
{
  startIOTservices();

  Lightbut.define_button(0, 0, D3, LOW, 1, 10);
  // Lightbut.define_light(0, D6, HIGH, true, false, 2, 4, 90, 1023, D7);
  Lightbut.define_light(0, D6, HIGH); /* IO light*/
  Lightbut.powerOn_powerFailure(0);

  Lightbut.define_button(1, 0, D4, LOW, 1, 10);
  Lightbut.define_light(1, D7, HIGH, true /* PWM */, true /* Dim */, 2, 4, 90, 1023); /* Dimmable PWM*/
  Lightbut.powerOn_powerFailure(0);
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
