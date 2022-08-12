#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"

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

const uint8_t LIGHT_PINS[] = {D5, D6, D7, D8};
const uint8_t BUTTON_PINS[] = {D3, D4, D3, D4};
const char *BUT_NAMES[] = {"L0", "L1", "L2", "L3"};

template <uint8_t N>
void LightButton<N>::sendMSG(oper_string &str, uint8_t i)
{
  print_OPERstring(str, i);
  notifyState(str.reason, i, str.offtime - str.ontime);
}

void start_LiButt()
{
  // Lightbut.define_button(0, 0, D3, LOW, 1, 10);
  // Lightbut.define_light(0, D6, HIGH); /* IO light*/
  // Lightbut.powerOn_powerFailure(0);
  // Lightbut.set_name(0, "Light_0");

  // if (numSW > 1)
  // {
  //   Lightbut.define_button(1 /* ID */, 0 /* Trig */, D5 /* io */, LOW, 1 /*def time min */, 10);
  //   Lightbut.define_light(1, D7, HIGH, true /* PWM */, true /* Dim */, 2 /* def Power */, 4 /* Max power steps */, 90, 1023); /* Dimmable PWM*/
  //   Lightbut.set_name(1, "LED_1");
  //   Lightbut.powerOn_powerFailure(1);
  // }
  for (uint8_t n = 0; n < numSW; n++)
  {
    Lightbut.define_button(n, 0, BUTTON_PINS[n], LOW, 1, 10, false);
    Lightbut.define_light(n, LIGHT_PINS[n], HIGH); /* IO light*/
    Lightbut.powerOn_powerFailure(n);
    Lightbut.set_name(n, BUT_NAMES[n]);
  }
}

void setup()
{
  startIOTservices();
  start_LiButt();
}

void loop()
{
  iot.looper();
  Lightbut.loop();
}
