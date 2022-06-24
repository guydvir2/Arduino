#include <myIOT2.h>
#include <Chrono.h>
#include <buttonPresses.h>

#define NUM_CHRONOS 2
Chrono chron_0(Chrono::SECONDS);
Chrono chron_1(Chrono::SECONDS);
Chrono *chronVector[NUM_CHRONOS] = {&chron_0, &chron_1};
buttonPresses inputButton;

/* ~~~~~~~~~~~~~~~~~~~~~~ Values get updated from parameter file ~~~~~~~~~~~~~~~~~~ */
uint8_t numSW = 1; /* Num of switches: 1 or 2 */
int PWM_res = 1023;
int sketch_JSON_Psize = 1250; /* Pass JSON size for Flash Parameter*/

bool useInputs[NUM_CHRONOS] = {false, false};
bool useIndicLED[NUM_CHRONOS] = {false, false}; /* use indication leds when ON*/

bool indic_ON[NUM_CHRONOS] = {true, true};
bool output_ON[NUM_CHRONOS] = {HIGH, HIGH};  /* OUTPUT when ON is HIGH or LOW */
bool inputPressed[NUM_CHRONOS] = {LOW, LOW}; /* High or LOW on button press */
bool OnatBoot[NUM_CHRONOS] = {false, false}; /* After reboot- On or Off */
bool outputPWM[NUM_CHRONOS] = {false, false};

uint8_t inputPin[NUM_CHRONOS] = {D6, 5};         /* IO for inputs */
uint8_t outputPin[NUM_CHRONOS] = {2, 2};         /* IO for outputs */
uint8_t indicPin[NUM_CHRONOS] = {4, 2};          /* IO for idication LEDS */
uint8_t defPWM[NUM_CHRONOS] = {2, 2};            /* Default PWM value for some cases not specified */
uint8_t limitPWM[NUM_CHRONOS] = {80, 80};        /* Limit total intensity, 1-100 */
char sw_names[NUM_CHRONOS][20] = {"SW1", "SW2"}; /* Name of each Switch, as shown on MQTT msg */

int maxTimeout[NUM_CHRONOS] = {600, 600};
int defaultTimeout[NUM_CHRONOS] = {30, 120};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "myIOT_settings.h"
#include "Schedualer_param.h"

int timeouts[NUM_CHRONOS] = {0, 0};

void startWatch(uint8_t i = 0)
{
  chronVector[i]->restart();
}
void stopWatch(uint8_t i = 0)
{
  chronVector[i]->restart();
  chronVector[i]->stop();
  timeouts[i] = 0;
}
long remainWatch(uint8_t i = 0)
{
  if (chronVector[i]->isRunning())
  {
    return chronVector[i]->elapsed() / 1000;
  }
  else
  {
    return 0;
  }
}
void addWatch(uint8_t i, int _add, const char *trigger)
{
  timeouts[i] += _add;
  notifyAdd(i, _add, trigger);

  if (!chronVector[i]->isRunning()) /* Case not ON */
  {
    ONcmd(i, timeouts[i]);
  }
}
void stopAllWatches()
{
  for (uint8_t i = 0; i < NUM_CHRONOS; i++)
  {
    stopWatch(i);
  }
}

void notifyAdd(uint8_t &i, int &_add, const char *trigger)
{
  char a[50];
  sprintf(a, "%s: [%s] Added [%d] seconds, total [%d] seconds", trigger, sw_names[i], _add, timeouts[i]);
  iot.pub_msg(a);
  Serial.println(a);
}
void notifyOFF(uint8_t &i, int &_elapsed, const char *trigger)
{
  char a[50];
  sprintf(a, "%s: [%s] Switched [Off] after [%d] seconds", trigger, sw_names[i], _elapsed);
  iot.pub_msg(a);
  Serial.println(a);
}
void notifyON(uint8_t &i, const char *trigger)
{
  char a[50];
  sprintf(a, "%s: [%s] Switched [On] for [%d] seconds", trigger, sw_names[i], timeouts[i]);
  iot.pub_msg(a);
  Serial.println(a);
}

void ONcmd(uint8_t i, uint8_t _TO, const char *trigger)
{
  if (!getHWstate(i))
  {
    if (_TO == 0)
    {
      timeouts[i] = defaultTimeout[i];
    }
    else
    {
      timeouts[i] = _TO;
    }
    startWatch(i);
    notifyON(i, trigger);
    HWswitch(i, true);
  }
}
void OFFcmd(uint8_t i, const char *trigger)
{
  if (getHWstate(i))
  {
    int x = chronVector[i]->elapsed();
    notifyOFF(i, x, trigger);
    stopWatch(i);
    HWswitch(i, false);
  }
}

void Chrono_looper()
{
  for (uint8_t i = 0; i < NUM_CHRONOS; i++)
  {
    if (chronVector[i]->isRunning() && chronVector[i]->hasPassed(timeouts[i]))
    {
      OFFcmd(i);
    }
  }
}

void Button_looper()
{
  if (useInputs[0])
  {
    uint8_t _read = inputButton.read();
    if (_read != 4)
    {
      if (_read == 0)
      {
        OFFcmd(0);
      }
      else if (_read == 1)
      {
        ONcmd(0);
      }
      else
      {
        yield();
      }
    }
  }
}
void startOUTPUTSio()
{
  analogWriteRange(PWM_res); /* PWM at ESP8266 */

  for (uint8_t x = 0; x < numSW; x++)
  {
    pinMode(outputPin[x], OUTPUT);
    if (OnatBoot[x])
    {
      digitalWrite(outputPin[x], output_ON[x]);
    }
    else
    {
      digitalWrite(outputPin[x], !output_ON[x]);
    }

    if (useIndicLED[x])
    {
      pinMode(indicPin[x], OUTPUT);
    }
  }
}
bool getHWstate(uint8_t &i)
{
  if (!outputPWM[i])
  {
    if (digitalRead(outputPin[i]) == output_ON[i]) // check if output's ON
    {
      return true; // mean ON regardless if on is low or high
    }
    else
    {
      return false; // return off
    }
  }
}
void HWswitch(uint8_t &i, bool state)
{
  if (!outputPWM[i])
  {
    if (state)
    {
      digitalWrite(outputPin[i], output_ON[i]);
    }
    else
    {
      digitalWrite(outputPin[i], !output_ON[i]);
    }
  }
}
void startINPUTSio()
{
  if (useInputs[0])
  {
    inputButton.pin0 = inputPin[0];
    inputButton.start();
  }
}

void setup()
{
  read_flashParameter();
  startOUTPUTSio();
  startINPUTSio();
  stopAllWatches();
  startIOTservices();
}
void loop()
{
  iot.looper();
  Chrono_looper();
  Button_looper();
}
