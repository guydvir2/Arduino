#include <myIOT2.h>
#include <Button2.h>
#include <Chrono.h>

#include "newClass.h"
#include "Scheduler.h"
#include "Chronos.h"
#include "myIOT_settings.h"
#include "getFlashParameters.h"
#include "Buttons.h"

void startOUTPUTSio()
{
  for (uint8_t x = 0; x < numSW; x++)
  {
    outputPWM[x] == true ? lightVector[x]->init(outputPin[x], PWM_res, dimPWM[x]) : lightVector[x]->init(outputPin[x], output_ON[x]);
    OnatBoot[x] == true ? lightVector[x]->turnON() : lightVector[x]->turnOFF();
    lightVector[x]->limitPWM = limitPWM[x];

    if (useIndicLED[x])
    {
      lightVector[x]->auxFlag(indicPin[x]);
    }
  }
}

void ONcmd(uint8_t i, uint8_t _TO, const char *trigger, uint8_t _PWMstep)
{
  if (!lightVector[i]->isON()) /* Enter when off or at PWM different PWM value */
  {
    _TO == 0 ? timeouts[i] = defaultTimeout[i] : timeouts[i] = _TO;
    startWatch(i);
    lightVector[i]->turnON(_PWMstep);
    notifyON(i, trigger);
  }
}
void OFFcmd(uint8_t i, const char *trigger)
{
  if (lightVector[i]->isON())
  {
    int x = chronVector[i]->elapsed();
    notifyOFF(i, x, trigger);
    stopWatch(i);
    lightVector[i]->turnOFF();
  }
}

void bootSummary()
{
  Serial.print("NumSW:\t\t");
  Serial.println(numSW);

  Serial.print("PWM_res:\t");
  Serial.println(PWM_res);

  Serial.print("sketch_JSON_Psize:\t");
  Serial.println(sketch_JSON_Psize);

  for (uint8_t i = 0; i < numSW; i++)
  {
    Serial.print(">>> ~~ Schedualer #");
    Serial.print(i);
    Serial.println(" ~~ <<<");

    Serial.print("sw_names:\t");
    Serial.println(sw_names[i]);

    Serial.print("useInputs:\t");
    Serial.println(useInputs[i]);

    Serial.print("inputPin:\t");
    Serial.println(inputPin[i]);

    Serial.print("inputPressed:\t");
    Serial.println(inputPressed[i]);

    Serial.print("trigType:\t");
    Serial.println(trigType[i]);

    Serial.print("useIndicLED:\t");
    Serial.println(useIndicLED[i]);

    Serial.print("indic_ON:\t");
    Serial.println(indic_ON[i]);

    Serial.print("indicPin:\t");
    Serial.println(indicPin[i]);

    Serial.print("outputPin:\t");
    Serial.println(outputPin[i]);

    Serial.print("output_ON:\t");
    Serial.println(output_ON[i]);

    Serial.print("OnatBoot:\t");
    Serial.println(OnatBoot[i]);

    Serial.print("outputPWM:\t");
    Serial.println(outputPWM[i]);

    Serial.print("defPWM:\t\t");
    Serial.println(defPWM[i]);

    Serial.print("limitPWM:\t");
    Serial.println(limitPWM[i]);

    Serial.print("dimPWM:\t\t");
    Serial.println(dimPWM[i]);

    Serial.print("maxPWMpresses:\t");
    Serial.println(maxPWMpresses[i]);

    Serial.print("maxTimeout:\t");
    Serial.println(maxTimeout[i]);

    Serial.print("defaultTimeout:\t");
    Serial.println(defaultTimeout[i]);
  }
}

void setup()
{
  read_flashParameter();
  startOUTPUTSio();
  startButtons();
  stopAllWatches();
  startIOTservices();
  bootSummary();
}
void loop()
{
  iot.looper();
  loopAllWatches();
  Button_looper();
  delay(100);
}
