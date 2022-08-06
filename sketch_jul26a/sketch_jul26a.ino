#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"


#define numSW 2
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

void start_LiButt()
{
  Lightbut.define_button(0, 0, D3, LOW, 1, 10);
  // Lightbut.define_light(0, D6, HIGH, true, false, 2, 4, 90, 1023, D7);
  Lightbut.define_light(0, D6, HIGH); /* IO light*/
  Lightbut.powerOn_powerFailure(0);

  Lightbut.define_button(1, 0, D4, LOW, 1, 10);
  Lightbut.define_light(1, D7, HIGH, true /* PWM */, true /* Dim */, 2, 4, 90, 1023); /* Dimmable PWM*/
  // Lightbut.powerOn_powerFailure(1);
}

/ ~~~~~~~~~~~ External trigger (not physical button, but MQTT cmd) for timeout and light ~~~~~~~~~~~~~
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
    Lightbut.stopTimeout_cb(TO, reason, i);  }
}
void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i)
{
  if (i < numSW)
  {
  //   if (lightOutputV[i]->isPWM())
  //   {
  //     if (timeoutButtonV[i]->getState()) /* if already ON */
  //     {
  //       if (lightOutputV[i]->turnON(step)) /* update PWM value */
  //       {
  //         notifyUpdatePWM(step, reason, i);
  //       }
  //     }
  //     else
  //     {
  //       Ext_trigger_ON(reason, timeoutButtonV[i]->defaultTimeout, step, i); /* if Off, turn ON with desired PWM value */
  //     }
  //     update_OperString(reason, i, true);
  //   }
  // }
}
void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i)
{
  // if (i < numSW)
  // {
  //   if (timeoutButtonV[i]->getState())
  //   {
  //     notifyAdd(timeAdd, reason, i);
  //   }
  //   timeoutButtonV[i]->addWatch(timeAdd, reason); /* update end time */
  //   update_OperString(reason, i, true);
  // }
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
