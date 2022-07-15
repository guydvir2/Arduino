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
timeoutButton timeoutButton_0;

struct oper_string
{
  bool state;     /* On or Off */
  uint8_t step;   /* Step, in case of PWM */
  uint8_t reason; /* What triggered the button */
  time_t ontime;  /* Start Clk */
  time_t offtime; /* Off Clk */
};

// ~~~~ All CBs are called eventually by timeoutButton instance OR external input (that call timeoutButton) ~~
void OFF_CB(uint8_t reason)
{
  notifyOFF(reason); /* First to display time elapsed */
  lightOutput_0.turnOFF();    /* and then turn off */
  update_OperString(reason, false);
}
void ON_CB(uint8_t reason)
{
  if (lightOutput_0.turnON(lightOutput_0.currentStep)) /* First to display power */
  {
    notifyON(reason);
    update_OperString(reason, true);
  }
}
void MULTP_CB(uint8_t reason)
{
  Serial.println(reason);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ External update (as MQTT) for timeout and light ~~~~~~~~~~~~~
void Ext_trigger_ON(uint8_t reason, int TO, uint8_t step)
{
  if (step != 0)
  {
    lightOutput_0.currentStep = step;
  }
  timeoutButton_0.ON_cb(TO, reason);
}
void Ext_trigger_OFF(uint8_t reason)
{
  timeoutButton_0.OFF_cb(reason);
}
void Ext_updatePWM_value(uint8_t reason, uint8_t step)
{
  if (lightOutput_0.isPWM())
  {
    if (timeoutButton_0.getState()) /* if already ON */
    {
      if (lightOutput_0.turnON(step)) /* update PWM value */
      {
        notifyUpdatePWM(step, reason);
      }
    }
    else
    {
      Ext_trigger_ON(reason, timeoutButton_0.defaultTimeout, step); /* if Off, turn ON with desired PWM value */
    }
  }
}
void Ext_addTime(uint8_t reason, int timeAdd)
{
  if (timeoutButton_0.getState())
  {
    notifyAdd(timeAdd, reason);
  }
  timeoutButton_0.addWatch(timeAdd, reason);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_OperString(uint8_t reason, bool state)
{
  oper_string str;

  str.state = timeoutButton_0.getState();
  str.ontime = iot.now();
  str.offtime = str.ontime + timeoutButton_0.timeout * 60;
  str.reason = reason;
  lightOutput_0.isPWM() ? str.step = lightOutput_0.currentStep : str.step = 0;

  File writefile = LITFS.open("filename", "w");
}
void init_TObutton()
{
  timeoutButton_0.ExtON_cb(ON_CB);
  timeoutButton_0.ExtOFF_cb(OFF_CB);
  timeoutButton_0.ExtMultiPress_cb(MULTP_CB);
  timeoutButton_0.begin(ButtonPin, TRIG_TYPE, TO_Button_ID);
}
void init_Light()
{
  if (PWM_OUTPUT)
  {
    lightOutput_0.init(LED_Pin, PWM_RES, PWM_DIM);
  }
  else
  {
    lightOutput_0.init(LED_Pin, HIGH);
  }

  if (USE_INDICATION)
  {
    lightOutput_0.auxFlag(IndicPin);
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
  timeoutButton_0.loop();
  iot.looper();
}
