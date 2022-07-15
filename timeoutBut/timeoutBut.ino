#include <myIOT2.h>
#include <timeoutButton.h>
#include <TurnOnLights.h>
#include "myIOT_settings.h"

#define LED_Pin D6
#define ButtonPin D5
#define IndicPin D7
#define TRIG_TYPE 0
#define TO_Button_ID 0
#define PWM_OUTPUT true
#define PWM_DIM true
#define PWM_RES 1023
#define USE_INDICATION true

myIOT2 iot;
TurnOnLights lightOutput_0;
TurnOnLights lightOutput_1;
// TurnOnLights *lightOutputV[2]; // = {&lightOutput_0, &lightOutput_1};

timeoutButton timeoutButton_0;
timeoutButton timeoutButton_1;
// timeoutButton *timeoutButtonV[2]; //={&timeoutButton_0 ,&timeoutButton_1};

struct oper_string
{
  bool state;     /* On or Off */
  uint8_t step;   /* Step, in case of PWM */
  uint8_t reason; /* What triggered the button */
  time_t ontime;  /* Start Clk */
  time_t offtime; /* Off Clk */
};

// ~~~~ All CBs are called eventually by timeoutButton instance OR external input (that call timeoutButton) ~~
void OFF_CB(uint8_t reason, uint8_t i)
{
  notifyOFF(reason);          /* First to display time elapsed */
  lightOutputV[i]->turnOFF(); /* and then turn off */
  update_OperString(reason, false);
}
void ON_CB(uint8_t reason, uint8_t i)
{
  if (lightOutputV[i]->turnON(lightOutputV[i]->currentStep)) /* First to display power */
  {
    notifyON(reason, uint8_t i);
    // update_OperString(reason, true);
  }
}
void MULTP_CB(uint8_t reason, uint8_t i)
{
  Serial.println(reason);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ External update (as MQTT) for timeout and light ~~~~~~~~~~~~~
void Ext_trigger_ON(uint8_t reason, int TO, uint8_t step, uint8_t i)
{
  if (step != 0)
  {
    lightOutputV[i]->currentStep = step;
  }
  timeoutButtonV[i]->ON_cb(TO, reason);
}
void Ext_trigger_OFF(uint8_t reason, uint8_t i)
{
  timeoutButtonV[i]->OFF_cb(reason);
}
void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i)
{
  if (lightOutputV[i]->isPWM())
  {
    if (timeoutButtonV[i]->getState()) /* if already ON */
    {
      if (lightOutputV[i]->turnON(step)) /* update PWM value */
      {
        notifyUpdatePWM(step, reason);
      }
    }
    else
    {
      Ext_trigger_ON(reason, timeoutButtonV[i]->defaultTimeout, step); /* if Off, turn ON with desired PWM value */
    }
  }
}
void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i)
{
  if (timeoutButtonV[i]->getState())
  {
    notifyAdd(timeAdd, reason);
  }
  timeoutButtonV[i]->addWatch(timeAdd, reason);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_OperString(uint8_t reason, bool state)
{
  oper_string str;

  str.state = timeoutButtonV[i]->getState();
  str.ontime = iot.now();
  str.offtime = str.ontime + timeoutButtonV[i]->timeout * 60;
  str.reason = reason;
  lightOutputV[i]->isPWM() ? str.step = lightOutputV[i]->currentStep : str.step = 0;

  File writefile = LITFS.open("filename", "w");
}
void init_TObutton(uint8_t i)
{
  timeoutButtonV[i]->ExtON_cb(ON_CB);
  timeoutButtonV[i]->ExtOFF_cb(OFF_CB);
  timeoutButtonV[i]->ExtMultiPress_cb(MULTP_CB);
  timeoutButtonV[i]->begin(ButtonPin, TRIG_TYPE, TO_Button_ID);
}
void init_Light(uint8_t i)
{
  if (PWM_OUTPUT)
  {
    lightOutputV[i]->init(LED_Pin, PWM_RES, PWM_DIM);
  }
  else
  {
    lightOutputV[i]->init(LED_Pin, HIGH);
  }

  if (USE_INDICATION)
  {
    lightOutputV[i]->auxFlag(IndicPin);
  }
}

void setup()
{
  startIOTservices();
  init_TObutton();
  init_Light();
}
void loop()
{
  timeoutButtonV[0]->loop();
  timeoutButtonV[1]->loop();
  iot.looper();
}
