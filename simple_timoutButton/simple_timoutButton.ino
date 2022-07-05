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
TurnOnLights led0;
timeoutButton TO_Button_0;

void OFF_CB(uint8_t reason)
{
  Serial.println("OFF");
  notifyOFF(reason); /* First to display time */
  led0.turnOFF();
}
void ON_CB(uint8_t reason)
{
  if (led0.turnON(led0.currentStep)) /* First to display power */
  {
    notifyON(reason);
  }
}
void ON_MQTT(uint8_t reason, int TO, uint8_t step)
{
  if (step != 0)
  {
    led0.currentStep = step;
  }
  TO_Button_0.ON_cb(TO, reason);
}
void OFF_MQTT(uint8_t reason)
{
  TO_Button_0.OFF_cb(reason);
}
void updatePWM(uint8_t reason, uint8_t step)
{
  if (TO_Button_0.getState())
  {
    if (led0.turnON(step))
    {
      notifyUpdatePWM(step, reason);
    }
  }
  else
  {
    ON_MQTT(reason, TO_Button_0.defaultTimeout, step);
  }
}
void addTime_MQTT(uint8_t reason, int timeAdd)
{
  if (TO_Button_0.getState())
  {
    notifyAdd(timeAdd, reason);
  }
  TO_Button_0.addWatch(timeAdd, reason);
}
void init_TObutton()
{
  TO_Button_0.ExtON_cb(ON_CB);
  TO_Button_0.ExtOFF_cb(OFF_CB);
  TO_Button_0.begin(ButtonPin, TRIG_TYPE, TO_Button_ID);
}
void init_Light()
{
  if (PWM_OUTPUT)
  {
    led0.init(LED_Pin, PWM_RES, PWM_DIM);
  }
  else
  {
    led0.init(LED_Pin, HIGH);
  }

  if (USE_INDICATION)
  {
    led0.auxFlag(IndicPin);
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
  TO_Button_0.loop();
  iot.looper();
}
