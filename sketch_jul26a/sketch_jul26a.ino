#include <myIOT2.h>
#include <timeoutButton.h>
#include "defs.h"

myIOT2 iot;
LightButton<MAX_SW> Lightbut;

#include "myIOT_flashParametrs.h"
#include "myIOT_settings.h"

template <uint8_t N>
void LightButton<N>::sendMSG(uint8_t i)
{
  oper_string str;
  readOperStr(str, i);
  notifyState(str.reason, i, str.offtime - str.ontime);
  
#if defined(DEBUG_MODE)
  Serial.print("~~~~~~~ OPER_STRING START #");
  Serial.print(i);
  Serial.println(" ~~~~~~~~~");

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
  Serial.println("~~~~~~~ OPER_STRING END ~~~~~~~~~~~~~");
#endif
}

void init_LightButton(JsonDocument &DOC)
{
  NUM_SWITCHES = DOC["numSW"];

  for (uint8_t n = 0; n < NUM_SWITCHES; n++)
  {
    oab[n] = DOC["useOnAtBoot"][n];
    powr[n] = DOC["usePowerRecovery"][n];
    strcpy(SwNames[n], DOC["nick"][n]);

    Lightbut.define_button(n, DOC["trig_type"][n], DOC["input_pins"][n], DOC["isPressed"][n], DOC["defTime"][n], DOC["maxTime"][n], DOC["useInput"][n]);
    Lightbut.define_light(n, DOC["output_pins"][n], DOC["isON"][n], DOC["usePWM"][n], DOC["isDimm"][n], DOC["defPWMstep"][n], DOC["maxPWMstep"][n], DOC["degPWM"][n], 1023, DOC["indic_pins"][n]);
  }
}
void turnON_afterBOOT() /* PowerON boot, Resume */
{
  /* Clock must be updated for those functions */
  for (uint8_t n = 0; n < NUM_SWITCHES; n++)
  {
    if (oab[n])
    {
      Serial.println("a");
      Lightbut.TurnON(0, 3, Lightbut.get_defcounter(n), n);
    }
    else if (powr[n])
    {
      Serial.println("b");
      Lightbut.powerOn_powerFailure(n);
    }
  }
}

void switches_setup_summary()
{
#if defined(DEBUG_MODE)
  char msg[100];

  Serial.println(" ~~~~~~~~~~~~~~~~~~~~~ SETUP SUMMARY ~~~~~~~~~~~~~~~~~~~~~");
  bool a;
#if defined(ESP8266)
  a = 0;
#else
  a = 1;
#endif
  sprintf(msg, "MCU:\t\t[%s]", a == 0 ? "ESP8266" : "ESP32");
  Serial.println(msg);
  sprintf(msg, "Max_Sw:\t[%d]", MAX_SW);
  Serial.println(msg);
  sprintf(msg, "inuse_Sw:\t[%d]", NUM_SWITCHES);
  Serial.println(msg);
  sprintf(msg, "free Heap:\t[%d]b", ESP.getFreeHeap());
  Serial.println(msg);

  for (uint8_t i = 0; i < NUM_SWITCHES; i++)
  {
    sprintf(msg, " ~~~~~~~ Switch #%d ~~~~~~~ ", i);
    Serial.println(msg);
    sprintf(msg, "INPUT IO:\t%d", i);
    Serial.println(msg);
    sprintf(msg, "OUTPUT IO:\t%d", i);
    Serial.println(msg);
    sprintf(msg, "Name:\t\t%s", SwNames[i]);
    Serial.println(msg);
    sprintf(msg, "useTimeout:\t%d", Lightbut.get_useTimeout(i));
    Serial.println(msg);
    sprintf(msg, "useInput:\t%d", Lightbut.get_useInput(i));
    Serial.println(msg);
    sprintf(msg, "useSave:\t%d", Lightbut.get_useSave(i));
    Serial.println(msg);
    sprintf(msg, "PWMlight:\t%d", Lightbut.isPwm(i));
    Serial.println(msg);
    sprintf(msg, "onAtBoot:\t%d", oab[i]);
    Serial.println(msg);
    sprintf(msg, "PowerRestore:\t%d", powr[i]);
    Serial.println(msg);

    Serial.println();
  }
#endif
}
void setup()
{
  Serial.begin(115200);
  startIOTservices();
  turnON_afterBOOT(); /* Must be placed after clock was updated */
  switches_setup_summary();
}
void loop()
{
  iot.looper();
  Lightbut.loop();
}
