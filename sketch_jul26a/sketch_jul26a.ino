#include <myIOT2.h>
#include <timeoutButton.h>

#define numSW 8

myIOT2 iot;
LightButton<numSW> Lightbut;

bool powr[numSW];
bool oab[numSW];
uint8_t NUM_SWITCHES;

#include "myIOT_flashParametrs.h"
#include "myIOT_settings.h"

void print_OPERstring(oper_string &str, uint8_t i)
{
  Serial.print(" +++++++++ OUTPUT OPER_STRING START #");
  Serial.print(i);
  Serial.println(" ++++++++++++");

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

void init_LightButton(JsonDocument &DOC)
{
  NUM_SWITCHES = DOC["numSW"];

  for (uint8_t n = 0; n < NUM_SWITCHES; n++)
  {
    powr[n] = DOC["usePowerRecovery"][n];
    oab[n] = DOC["useOnAtBoot"][n];

    Lightbut.define_button(n, DOC["trig_type"][n], DOC["input_pins"][n], DOC["isPressed"][n], DOC["defTime"][n], DOC["maxTime"][n], DOC["useInput"][n]);
    Lightbut.define_light(n, DOC["output_pins"][n], DOC["isON"][n], DOC["usePWM"][n], DOC["isDimm"][n], DOC["defPWMstep"][n], DOC["maxPWMstep"][n], DOC["degPWM"][n], 1023, DOC["indic_pins"][n]);
    Lightbut.set_name(n, DOC["nick"][n].as<const char *>());
  }
}
void postBoot_LightButton()
{
  /* Clock must be updated for those functions */
  for (uint8_t n = 0; n < NUM_SWITCHES; n++)
  {
    if (oab[n])
    {
      Lightbut.TurnON(0, 3, Lightbut.get_defcounter(n), n);
    }
    else if (powr[n])
    {
      Lightbut.powerOn_powerFailure(n);
    }
  }
}

void setup()
{
  startIOTservices();
  postBoot_LightButton(); /* Must be placed after clock was updated */
}
void loop()
{
  iot.looper();
  Lightbut.loop();
}
