#include <myIOT2.h>
#include <Button2.h>
#include <Chrono.h>

#include "Scheduler.h"
#include "Chronos.h"
#include "myIOT_settings.h"
#include "getFlashParameters.h"
#include "Buttons.h"
#include "HWswitch.h"

class PWMlight
{
public:
  int PWMval = 0;
  int PWMres = 1023;
  int defPWMvalue = (int)(PWMres * 0.7);

  uint8_t pwmPin = 255;
  uint8_t defStep = 2;
  uint8_t maxSteps = 3;
  uint8_t currentStep = 0;

public:
  PWMlight()
  {
  }
  void init(uint8_t pin, int res)
  {
    pwmPin = pin;
    PWMres = res;
    analogWriteRange(PWMres);
    pinMode(pwmPin, OUTPUT);
  }

  void turnOFF()
  {
    currentStep = 0;
    _setPWM(0);
  }
  void turnON(int val = 0)
  {
    if (val == 0)
    {
      _setPWM(defPWMvalue);
    }
    else
    {
      if (_validatePWM(val))
      {
        _setPWM(val);
      }
    }
  }
  void turnONstep(int8_t step)
  {
    if (_validateStep(step))
    {
      currentStep = step;
      _setPWM(_step2Value(step));
    }
  }

  void turnDim(int val)
  {
    int PWMstep_change = 1;

    val > PWMval ? PWMstep_change = PWMstep_change : PWMstep_change = -PWMstep_change;
    while (abs(val - PWMval) >= abs(PWMstep_change))
    {
      _setPWM(PWMval + PWMstep_change);
      delay(1);
    }
  }
  void turnDimstep(int8_t step)
  {
    if (_validateStep(step))
    {
      turnDim(_step2Value(step));
      currentStep = step;
    }
  }
  bool isON()
  {
    return PWMval > 0;
  }

private:
  void _setPWM(int val)
  {
    if (_validatePWM(val))
    {
      PWMval = val;
      analogWrite(pwmPin, val);
    }
  }
  bool _validatePWM(int val)
  {
    if (val >= 0 && val <= PWMres)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  bool _validateStep(int step)
  {
    if (step >= 0 && step <= maxSteps)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  int _step2Value(uint8_t step)
  {
    return (int)(step * PWMres / maxSteps);
  }
};

PWMlight PWMled;

void ONcmd(uint8_t i, uint8_t _TO, const char *trigger, uint8_t _pwmVal)
{
  if (!getHWstate(i) || (outputPWM[i] && _pwmVal != lastPWMvalue[i])) /* Enter when off or at PWM different PWM value */
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
    HWswitch(i, true, _pwmVal);
  }
}
void OFFcmd(uint8_t i, const char *trigger)
{
  if (getHWstate(i))
  {
    int x = chronVector[i]->elapsed();
    notifyOFF(i, x, trigger);
    stopWatch(i);
    HWswitch(i, false, 0);
  }
}

void bootSummary()
{
  Serial.print("NumSW:\t");
  Serial.println(numSW);
  Serial.print("PWM_res:\t");
  Serial.println(sketch_JSON_Psize);

  for (uint8_t i = 0; i < numSW; i++)
  {
    Serial.print(">>> ~~~~ Schedualer #");
    Serial.print(i);
    Serial.println(" ~~~~ <<<");

    Serial.print("useInputs:\t");
    Serial.println(useInputs[i]);

    Serial.print("useIndicLED:\t");
    Serial.println(useIndicLED[i]);

    Serial.print("indic_ON:\t");
    Serial.println(indic_ON[i]);

    Serial.print("output_ON:\t");
    Serial.println(output_ON[i]);

    Serial.print("inputPressed:\t");
    Serial.println(inputPressed[i]);

    Serial.print("OnatBoot:\t");
    Serial.println(OnatBoot[i]);

    Serial.print("outputPWM:\t");
    Serial.println(outputPWM[i]);

    Serial.print("trigType:\t");
    Serial.println(trigType[i]);

    Serial.print("inputPin:\t");
    Serial.println(inputPin[i]);

    Serial.print("outputPWM:\t");
    Serial.println(outputPWM[i]);

    Serial.print("outputPin:\t");
    Serial.println(outputPin[i]);

    Serial.print("indicPin:\t");
    Serial.println(indicPin[i]);

    Serial.print("defPWM:\t");
    Serial.println(defPWM[i]);

    Serial.print("indicPin:\t");
    Serial.println(indicPin[i]);

    Serial.print("limitPWM:\t");
    Serial.println(limitPWM[i]);

    Serial.print("sw_names:\t");
    Serial.println(sw_names[i]);

    Serial.print("maxTimeout:\t");
    Serial.println(maxTimeout[i]);

    Serial.print("defaultTimeout:\t");
    Serial.println(defaultTimeout[i]);
  }
}

void setup()
{
  PWMled.init(D1, 1023);

  Serial.begin(115200);
  PWMled.maxSteps = 10;
  // read_flashParameter();
  // startOUTPUTSio();
  // startButtons();
  // startIOTservices();
  // bootSummary();
}
void loop()
{
  for (uint8_t i = 0; i <= PWMled.maxSteps; i++)
  {
    Serial.println(i);
    Serial.print("on: ");
    Serial.println(PWMled.isON());
    PWMled.turnDimstep(i);
    delay(1000);
  }

  // PWMled.turnOFF();
  // delay(1500);
  // iot.looper();
  // loopAllWatches();
  // Button_looper();
}
