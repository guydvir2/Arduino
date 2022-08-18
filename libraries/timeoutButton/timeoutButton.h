#ifndef TOB_h
#define TOB_h

#include <Arduino.h>
#include <Button2.h>
#include <Chrono.h>
#include <TurnOnLights.h>
#include <FS.h>

#if defined(ESP32)
#include "LITTLEFS.h"
#define LITFS LITTLEFS
#elif defined(ESP8266)
#include <LittleFS.h>
#define LITFS LittleFS
#endif

#define conv2Minute(t) t * 60
struct oper_string
{
    bool state;     /* On or Off */
    uint8_t step;   /* Step, in case of PWM */
    uint8_t reason; /* What triggered the button */
    time_t ontime;  /* Start Clk */
    time_t offtime; /* Off Clk */
};
enum SWITCH_TYPES : const uint8_t
{
    MOMENTARY,
    ON_OFF,
    TRIGGER_SERNSOR,
    MULTI_PRESS
};
enum REASONS : const uint8_t
{
    TIMEOUT,
    BUTTON,
    MQTT,
    PWRON,
    REBOOT
};

class timeoutButton
{
private:
    char _operfile[15];
    bool _useInput = false;
    unsigned long _lastPress = 0;

private:
    Chrono chrono;
    Button2 button;

public:
    int timeout = 0;
    int maxTimeout = 500;
    int defaultTimeout = 1;
    int time2Repress = 2000; // millis between valid repress counts

    bool flag2ON = false;
    bool flag2OFF = false;

    uint8_t Id = 0;
    uint8_t Inpin = 255;
    uint8_t trigTYPE = 0;
    uint8_t pressCounter = 0;

    oper_string OPERstring = {false, 0, 0, 0, 0};

protected:
    const char *ver = "timeouter_v0.3";

private:
    void _commonBegin(uint8_t id);
    void _init_button();
    void _Button_looper();
    void _ON_OFF_on_handle(Button2 &b);
    void _Momentary_handle(Button2 &b);
    void _ON_OFF_off_handle(Button2 &b);
    void _TrigSensor_handler(Button2 &b);
    void _MultiPress_handler(Button2 &b);

    void _stopClock();
    void _loopClock();
    void _startClock();
    void _init_chrono();

public:
    timeoutButton();
    void begin(uint8_t id); /* Not using button */
    void begin(uint8_t pin, uint8_t trigType, uint8_t id = 0);
    void addClock(int _add, uint8_t reason);

    void loop();
    bool getState();
    unsigned int remainClock();

    void stopTimeout_cb(uint8_t reason);
    void startTimeout_cb(int _TO, uint8_t reason);

    void save_OperStr(oper_string &str);
    bool read_OperStr(oper_string &str);
    void print_OPERstring(oper_string &str);
};

template <uint8_t N>
class LightButton
{
private:
    TurnOnLights _light[N];
    timeoutButton _Button[N];

private:
    void _init_onAtBoot(uint8_t i);
    void _turnONlights(uint8_t i);
    void _turnOFFlights(uint8_t i);
    void _newActivity_handler(uint8_t i);
#define _max_name_len 15

public:
    LightButton();
    char names[N][_max_name_len];

    void loop();
    bool isPwm(uint8_t i);
    void set_name(uint8_t i, const char *n);
    uint8_t get_counter(uint8_t i);
    uint8_t get_maxcounter(uint8_t i);
    uint8_t get_defcounter(uint8_t i);
    void powerOn_powerFailure(uint8_t i);
    void sendMSG(oper_string &str, uint8_t i);
    void define_button(uint8_t i, uint8_t trig, uint8_t pin, bool inputPressed = LOW, int defMinutes = 120, int maxMinutes = 360, bool useButton = true);
    void define_light(uint8_t i, uint8_t pin, bool outputON = HIGH, bool isPWM = false, bool isDim = true, uint8_t defPWM = 2, uint8_t max_pcount = 3, uint8_t limitPWM = 70, int PWMr = 1023, uint8_t indicPin = 255);

    // ~~~~~~~~ Belongs to Button Class ~~~~~
    bool getState(uint8_t i);
    int get_timeout(uint8_t i);
    unsigned int remainClock(uint8_t i);
    void addClock(int _add, uint8_t reason, uint8_t i);
    void set_PWM(uint8_t i, uint8_t count);
    void TurnOFF(uint8_t reason, uint8_t i);
    void TurnON(int _TO, uint8_t reason, uint8_t step, uint8_t i);
};

template <uint8_t N>
LightButton<N>::LightButton() : _Button(), _light()
{
}

template <uint8_t N>
void LightButton<N>::define_button(uint8_t i, uint8_t trig, uint8_t pin, bool inputPressed, int defMinutes, int maxMinutes, bool useButton)
{
    _Button[i].Id = i;
    _Button[i].trigTYPE = trig;
    _Button[i].Inpin = pin;
    _Button[i].defaultTimeout = conv2Minute(defMinutes);
    _Button[i].maxTimeout = conv2Minute(maxMinutes);

    if (!useButton)
    {
        _Button[i].begin(i); /* Not using button */
    }
    else
    {
        _Button[i].begin(pin, trig, i); /* Using button */
    }
}

template <uint8_t N>
void LightButton<N>::define_light(uint8_t i, uint8_t pin, bool outputON, bool isPWM, bool isDim, uint8_t defPWM, uint8_t max_pcount, uint8_t limitPWM, int PWMr, uint8_t indicPin)
{
    _light[i].Pin = pin;
    _light[i].isON = outputON;
    _light[i].useDim = isDim;
    _light[i].PWMmode = isPWM;

    _light[i].auxPin = indicPin;
    _light[i].defStep = defPWM;
    _light[i].maxSteps = max_pcount;
    _light[i].limitPWM = limitPWM;
    _light[i].PWMres = PWMr;

    if (!isPWM)
    {
        _light[i].init(pin, outputON); /* GPIO output */
    }
    else
    {
        _light[i].init(pin, PWMr, isDim); /* PWM output */
    }
    _light[i].auxFlag(indicPin); /* init if pin != 255 */
}

template <uint8_t N>
void LightButton<N>::_newActivity_handler(uint8_t i)
{
    // if (_Button[i].OPERstring.state) /* ON */
    // {
    //     if (_light[i].isPWM())
    //     {
    //         if (_Button[i].pressCounter <= _light[i].maxSteps) /* MultiPresses */
    //         {
    //             _turnONlights(i);
    //         }
    //         else
    //         {
    //             _turnOFFlights(i);
    //         }
    //     }
    //     else
    //     {
    //         _turnONlights(i);
    //     }
    // }
    // else
    // {
    //     _turnOFFlights(i);
    // }
    // sendMSG(_Button[i].OPERstring, i);

    // _Button[i].newMSG = false;
}

template <uint8_t N>
void LightButton<N>::loop()
{
    for (uint8_t i = 0; i < N; i++)
    {
        if (_Button[i].flag2ON)
        {
            if (_light[i].isPWM())
            {
                if (_Button[i].pressCounter <= _light[i].maxSteps) /* MultiPresses */
                {
                    _turnONlights(i);
                }
                else
                {
                    _turnOFFlights(i);
                }
            }
            else
            {
                _turnONlights(i);
            }
            _Button[i].flag2ON = false;
            sendMSG(_Button[i].OPERstring, i);
        }
        else if (_Button[i].flag2OFF)
        {
            _turnOFFlights(i);
            _Button[i].flag2OFF = false;
            sendMSG(_Button[i].OPERstring, i);
        }

        _Button[i].loop();
    }
}

template <uint8_t N>
unsigned int LightButton<N>::remainClock(uint8_t i)
{
    return _Button[i].remainClock();
}

template <uint8_t N>
void LightButton<N>::addClock(int _add, uint8_t reason, uint8_t i)
{
    _Button[i].addClock(_add, reason);
}

template <uint8_t N>
bool LightButton<N>::getState(uint8_t i)
{
    return _Button[i].getState();
}

template <uint8_t N>
uint8_t LightButton<N>::get_counter(uint8_t i)
{
    return _Button[i].pressCounter;
}

template <uint8_t N>
uint8_t LightButton<N>::get_defcounter(uint8_t i)
{
    return _light[i].defStep;
}

template <uint8_t N>
uint8_t LightButton<N>::get_maxcounter(uint8_t i)
{
    return _light[i].maxSteps;
}

template <uint8_t N>
int LightButton<N>::get_timeout(uint8_t i)
{
    return _Button[i].timeout;
}

template <uint8_t N>
bool LightButton<N>::isPwm(uint8_t i)
{
    return _light[i].PWMmode;
}

template <uint8_t N>
void LightButton<N>::TurnOFF(uint8_t reason, uint8_t i)
{
    if (_Button[i].getState() && _light[i].is_ON()) /* When Light and timer works together */
    {
        _Button[i].stopTimeout_cb(reason);
    }
    else if (!_Button[i].getState() && _light[i].is_ON()) /* in case that Light is on and timer is not running */
    {
        _light[i].turnOFF();
    }
}

template <uint8_t N>
void LightButton<N>::TurnON(int _TO, uint8_t reason, uint8_t step, uint8_t i)
{
    if (_Button[i].getState() || !_light[i].is_ON())
    {
        TurnOFF(reason, i);
    }
    _Button[i].pressCounter = step;
    _Button[i].startTimeout_cb(conv2Minute(_TO), reason);
}

template <uint8_t N>
void LightButton<N>::powerOn_powerFailure(uint8_t i)
{
    if (_Button[i].read_OperStr(_Button[i].OPERstring))
    {
        if (_Button[i].OPERstring.state == true)
        {
            if (_Button[i].OPERstring.offtime > time(nullptr))
            {
                _Button[i].startTimeout_cb(_Button[i].OPERstring.offtime - time(nullptr), REBOOT);
            }
            else
            {
                TurnOFF(REBOOT, i);
            }
        }
    }
}

template <uint8_t N>
void LightButton<N>::set_PWM(uint8_t i, uint8_t count)
{
    _Button[i].pressCounter = count;
    _light[i].currentStep = count;
    _light[i].turnON(count);
}

template <uint8_t N>
void LightButton<N>::set_name(uint8_t i, const char *n)
{
    strlcpy(names[i], n, _max_name_len);
}

template <uint8_t N>
void LightButton<N>::_turnONlights(uint8_t i)
{
    if (_Button[i].pressCounter == 0)
    {
        _Button[i].pressCounter = get_defcounter(i);
    }
    _light[i].turnON(_Button[i].pressCounter);
    _Button[i].OPERstring.step = _Button[i].pressCounter;
    _Button[i].save_OperStr(_Button[i].OPERstring);
}

template <uint8_t N>
void LightButton<N>::_turnOFFlights(uint8_t i)
{
    if (_light[i].is_ON())
    {
        _light[i].turnOFF();
        _Button[i].stopTimeout_cb(BUTTON);
    }
}

#endif
