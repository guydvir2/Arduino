class TurnLightsON
{
#define TURN_ON digitalWrite(Pin, _isON)
#define TURN_OFF digitalWrite(Pin, !_isON)
#define IS_OUTPUT_ON digitalRead(Pin) == _isON

private:
    bool _isON = HIGH;
    bool _PWMmode = true;
    bool _useDim = false; /* PWM only */

public:
    int PWMval = 0;
    int PWMres = 1023;
    int defPWMvalue = (int)(PWMres * 0.7);

    uint8_t Pin = 255;
    uint8_t defStep = 2;
    uint8_t maxSteps = 3;
    uint8_t currentStep = 0;

protected:
    const char *ver = "turnLight_v0.2";

public:
    TurnLightsON()
    {
    }
    ~TurnLightsON()
    {
        delete[] ver;
    }
    void init(uint8_t pin, int res, bool usedim) /* PWM */
    {
        Pin = pin;
        PWMres = res;
        _PWMmode = true;
        _useDim = usedim;
#if defined(ESP8266)
        analogWriteRange(PWMres);
#endif
        pinMode(Pin, OUTPUT);
    }
    void init(uint8_t pin, bool isON) /* GPIO */
    {
        Pin = pin;
        _PWMmode = false;
        _isON = isON;
        pinMode(Pin, OUTPUT);
    }

    void turnOFF() /* PWM & GPIO */
    {
        if (_PWMmode)
        {
            currentStep = 0;
            _setPWM(0);
        }
        else
        {
            TURN_OFF;
        }
    }
    void turnON(uint8_t step = 0) /* PWM & GPIO */
    {
        if (_PWMmode)
        {
            step == 0 ? _setPWM(defPWMvalue) : _setPWM(_step2Value(step));
        }
        else
        {
            TURN_ON;
        }
    }
    void PWMvalue(int val)
    {
        if (_isValidPWM(val) && _PWMmode)
        {
            _setPWM(val);
        }
        else
        {
            yield();
        }
    }
    bool isON()
    {
        if (_PWMmode)
        {
            return PWMval > 0;
        }
        else
        {
            return IS_OUTPUT_ON;
        }
    }

private:
    void _setPWM(int val)
    {
        if (_isValidPWM(val))
        {
            if (_useDim)
            {
                _Dim2Value(val);
            }
            else
            {
                analogWrite(Pin, val);
            }
            PWMval = val;
        }
    }
    void _Dim2Value(int &val)
    {
        int PWMstep_change = 1;

        val > PWMval ? PWMstep_change = PWMstep_change : PWMstep_change = -PWMstep_change;
        while (abs(val - PWMval) >= abs(PWMstep_change))
        {
            analogWrite(Pin, PWMval + PWMstep_change);
            delay(1);
        }
    }
    bool _isValidPWM(int val)
    {
        return (val >= 0 && val <= PWMres);
    }
    bool _isValidStep(int step)
    {
        return (step >= 0 && step <= maxSteps);
    }
    int _step2Value(uint8_t step)
    {
        return (int)(step * PWMres / maxSteps);
    }
};