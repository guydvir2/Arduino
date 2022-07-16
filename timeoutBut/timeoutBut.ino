#include <myIOT2.h>
#include <timeoutButton.h>
#include <TurnOnLights.h>

myIOT2 iot;
TurnOnLights lightOutput_0;
TurnOnLights lightOutput_1;
TurnOnLights *lightOutputV[2] = {&lightOutput_0, &lightOutput_1};

timeoutButton timeoutButton_0;
timeoutButton timeoutButton_1;
timeoutButton *timeoutButtonV[2] = {&timeoutButton_0, &timeoutButton_1};

//  ±±±±±±±±±±±±±±± Sketch variable - updates from flash ±±±±±±±±±±±±±±±±±±
uint8_t numSW = 2;

bool OnatBoot[2] = {true, false};
bool useInput[2] = {true, true};
bool outputPWM[2] = {true, true};
bool useIndicLED[2] = {false, false};
bool dimmablePWM[2] = {true, false};

bool output_ON[2] = {HIGH, HIGH};
bool inputPressed[2] = {LOW, LOW};

uint8_t trigType[2] = {0, 1};
uint8_t inputPin[2] = {D3, D5};
uint8_t outputPin[2] = {D6, D7};
uint8_t indicPin[2] = {D4, D8};

int def_TO_minutes[2] = {300, 300};
int maxON_minutes[2] = {1000, 1000};

uint8_t defPWM[2] = {2, 2};
uint8_t max_pCount[2] = {3, 3};
uint8_t limitPWM[2] = {80, 80};
uint8_t PWM_res = 1023;

int sketch_JSON_Psize = 1250;
const char *v_file = "0.83";

char sw_names[2][10] = {"led0", "led1"};
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

#include "myIOT_settings.h"

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
  notifyOFF(reason, i);       /* First to display time elapsed */
  lightOutputV[i]->turnOFF(); /* and then turn off */
  update_OperString(reason, false, i);
}
void ON_CB(uint8_t reason, uint8_t i)
{
  if (lightOutputV[i]->turnON(lightOutputV[i]->currentStep)) /* First to display power */
  {
    notifyON(reason, i);
    update_OperString(reason, true, i);
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
  if (i < numSW)
  {
    if (step != 0)
    {
      lightOutputV[i]->currentStep = step;
    }
    timeoutButtonV[i]->ON_cb(TO, reason);
  }
}
void Ext_trigger_OFF(uint8_t reason, uint8_t i)
{
  if (i < numSW)
  {
    timeoutButtonV[i]->OFF_cb(reason);
  }
}
void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i)
{
  if (i < numSW)
  {
    if (lightOutputV[i]->isPWM())
    {
      if (timeoutButtonV[i]->getState()) /* if already ON */
      {
        if (lightOutputV[i]->turnON(step)) /* update PWM value */
        {
          notifyUpdatePWM(step, reason, i);
        }
      }
      else
      {
        Ext_trigger_ON(reason, timeoutButtonV[i]->defaultTimeout, step, i); /* if Off, turn ON with desired PWM value */
      }
    }
  }
}
void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i)
{
  if (i < numSW)
  {
    if (timeoutButtonV[i]->getState())
    {
      notifyAdd(timeAdd, reason, i);
    }
    timeoutButtonV[i]->addWatch(timeAdd, reason);
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void onAtBoot()
{
  if (OnatBoot[0])
  {
    Ext_trigger_ON(3, 0);
  }
  if (OnatBoot[1])
  {
    Ext_trigger_ON(3, 1);
  }
}
void update_OperString(uint8_t reason, bool state, uint8_t i)
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
  if (useInput[i])
  {
    timeoutButtonV[i]->begin(inputPin[i], trigType[i], i);
  }
  else
  {
    timeoutButtonV[i]->begin(i);
  }
}
void init_Light(uint8_t i)
{
  if (outputPWM[i])
  {
    lightOutputV[i]->init(outputPin[i], PWM_res, dimmablePWM[i]);
  }
  else
  {
    lightOutputV[i]->init(outputPin[i], output_ON[i]);
  }

  if (useIndicLED[i])
  {
    lightOutputV[i]->auxFlag(indicPin[i]);
  }
}

void setup()
{
  startIOTservices();
  init_TObutton(0);
  init_Light(0);
  onAtBoot();

  if (numSW == 2)
  {
    init_TObutton(1);
    init_Light(1);
  }
}
void loop()
{
  timeoutButtonV[0]->loop();
  if (numSW == 2)
  {
    timeoutButtonV[1]->loop();
  }
  iot.looper();
}
