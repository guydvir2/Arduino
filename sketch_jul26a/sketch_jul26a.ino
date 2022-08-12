#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"

#define numSW 2

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
  notifyState(str.reason, i, str.offtime - str.ontime);
}

void start_LiButt()
{

  Lightbut.define_button(0, 0, D3, LOW, 1, 10);
  // Lightbut.define_light(0, D6, HIGH, true, false, 2, 4, 90, 1023, D7);
  Lightbut.define_light(0, D6, HIGH); /* IO light*/
  Lightbut.powerOn_powerFailure(0);
  Lightbut.set_name(0, "Light_0");

  if (numSW > 1)
  {
    Lightbut.define_button(1 /* ID */, 0 /* Trig */, D5 /* io */, LOW, 1 /*def time min */, 10);
    Lightbut.define_light(1, D7, HIGH, true /* PWM */, true /* Dim */, 2 /* def Power */, 4 /* Max power steps */, 90, 1023); /* Dimmable PWM*/
    Lightbut.set_name(1, "LED_1");
    Lightbut.powerOn_powerFailure(1);
  }
}

// ~~~~~~~~~~~ External trigger (not physical button, but MQTT cmd) for timeout and light ~~~~~~~~~~~~~
void Ext_trigger_ON(uint8_t reason, int TO, uint8_t step, uint8_t i)
{
  if (i < numSW)
  {
    Lightbut.Ext_setCounter(i, step);
    Lightbut.startTimeout_cb(TO, reason, i);
  }
}
void Ext_trigger_OFF(uint8_t reason, uint8_t i)
{
  if (i < numSW)
  {
    Lightbut.Ext_setCounter(i, 0);
    Lightbut.stopTimeout_cb(reason, i);
  }
}
void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i)
{
  if (i < numSW)
  {
    if (Lightbut.getState(i) == true) /* if already ON */
    {
      Lightbut.Ext_setCounter(i, step);
    }
    else
    {
      Ext_trigger_ON(reason, 0, step, i); /* if Off, turn ON with desired PWM value */
    }
  }
}
void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i)
{
  if (i < numSW)
  {
    Lightbut.addClock(conv2Minute(timeAdd), reason, i);
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  startIOTservices();
  start_LiButt();
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
