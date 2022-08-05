#include "TurnOnLights.h"

TurnOnLights::TurnOnLights()
{
}
void TurnOnLights::init(uint8_t pin, int res, bool usedim) /* PWM */
{
    Pin = pin;
    PWMres = res;
    PWMmode = true;
    useDim = usedim;
#if defined(ESP8266)
    analogWriteRange(PWMres);
#endif
    pinMode(Pin, OUTPUT);
}
void TurnOnLights::init(uint8_t pin, bool _isON) /* GPIO */
{
    Pin = pin;
    PWMmode = false;
    isON = _isON;
    pinMode(Pin, OUTPUT);
}
void TurnOnLights::auxFlag(uint8_t pin)
{
    if (pin != 255)
    {
        auxPin = pin;
        pinMode(auxPin, OUTPUT);
        _useAuxFlag = true;
    }
}

bool TurnOnLights::turnOFF() /* PWM & GPIO */
{
    if (PWMmode)
    {
        currentStep = 0;
        if (_setPWM(0))
        {
            INDIC_OFF;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        TURN_OFF;
        INDIC_OFF;
        return 1;
    }
}
bool TurnOnLights::turnON(uint8_t step) /* PWM & GPIO */
{
    if (PWMmode)
    {
        step == 0 ? currentStep = defStep : currentStep = step;
        if (_setPWM(_step2Value(currentStep)))
        {
            INDIC_ON;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        TURN_ON;
        INDIC_ON;
        return 1;
    }
}
bool TurnOnLights::PWMvalue(int val)
{
    if (_isValidPWM(val) && PWMmode)
    {
        return _setPWM(val);
    }
    else
    {
        return 0;
    }
}
void TurnOnLights::blink(uint8_t blinks, int _delay)
{
    turnOFF();
    for (uint8_t i = 0; i < blinks; i++)
    {
        turnON();
        delay(_delay / 2);
        turnOFF();
        delay(_delay / 2);
    }
}

bool TurnOnLights::is_ON()
{
    if (PWMmode)
    {
        return PWMval > 0;
    }
    else
    {
        return IS_OUTPUT_ON;
    }
}
bool TurnOnLights::isPWM()
{
    return PWMmode;
}

bool TurnOnLights::_setPWM(int val)
{
    if (_isValidPWM(val))
    {
        if (useDim)
        {
            _Dim2Value(val);
        }
        else
        {
            analogWrite(Pin, (val * limitPWM) / 100);
            PWMval = val;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}
void TurnOnLights::_Dim2Value(int val)
{
    int PWMstep_change = dimDelay;

    val > PWMval ? PWMstep_change = PWMstep_change : PWMstep_change = -PWMstep_change;
    while (abs(val - PWMval) >= abs(PWMstep_change))
    {
        PWMval += PWMstep_change;
        analogWrite(Pin, (PWMval * limitPWM) / 100);
        delayMicroseconds(dimDelay * 5);
    }
}
bool TurnOnLights::_isValidPWM(int val)
{
    return (val >= 0 && val <= PWMres);
}
bool TurnOnLights::_isValidStep(int step)
{
    return (step >= 0 && step <= maxSteps);
}
int TurnOnLights::_step2Value(uint8_t step)
{
    return (int)((step * PWMres) / maxSteps);
}
