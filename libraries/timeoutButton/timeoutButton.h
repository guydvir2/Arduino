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
#define conv2Minute(t) t * 10

private:
    const char *INPUT_ORG[5] = {"Timeout", "Button", "MQTT", "PowerON", "Resume Reboot"};

    unsigned long _lastPress = 0;
    bool _useInput = false;
    char _operfile[15];

private:
    Chrono chrono;
    Button2 button;

public:
    int timeout = 0;
    int maxTimeout = 500;
    int defaultTimeout = 1;
    int time2Repress = 2000; // millis between valid repress counts
    oper_string OPERstring = {false, 0, 0, 0, 0};
    bool newMSG = false;
    uint8_t pressCounter = 0;
    uint8_t trigTYPE = 0;
    uint8_t Id = 0;
    uint8_t Inpin = 255;

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
    void read_OperStr(oper_string &str);
    void print_OPERstring(oper_string &str);
};

template <uint8_t N>
class LightButton
{
public:
    oper_string *OPstr = &Button[N].OPERstring;

private:
    timeoutButton Button[N];
    TurnOnLights light[N];

private:
    void _newActivity_handler(uint8_t i);
    void _init_onAtBoot(uint8_t i);
    void _turnONlights(uint8_t i);
    void _turnOFFlights(uint8_t i);

public:
    LightButton();
    void sendMSG(oper_string &str);
    void define_button(uint8_t i, uint8_t trig, uint8_t pin, bool inputPressed = LOW, int defMinutes = 120, int maxMinutes = 360);
    void define_light(uint8_t i, uint8_t pin, bool outputON = HIGH, bool isPWM = false, bool isDim = true, uint8_t defPWM = 2, uint8_t max_pcount = 3, uint8_t limitPWM = 70, int PWMr = 1023, uint8_t indicPin = 255);

    // ~~~~~~~~ Belongs to Button Class ~~~~~
    void loop();
    bool getState(uint8_t i);
    void begin();
    unsigned int remainClock(uint8_t i);
    void stopTimeout_cb(uint8_t reason, uint8_t i);
    void startTimeout_cb(int _TO, uint8_t reason, uint8_t i);
};

template <uint8_t N>
LightButton<N>::LightButton() : Button(), light()
{
}

template <uint8_t N>
void LightButton<N>::define_button(uint8_t i, uint8_t trig, uint8_t pin, bool inputPressed, int defMinutes, int maxMinutes)
{
    Button[i].Id = i;
    Button[i].trigTYPE = trig;
    Button[i].Inpin = pin;
    Button[i].defaultTimeout = defMinutes;
    Button[i].maxTimeout = maxMinutes;

    // Button[i].begin(_buttonID);                     /* Not using button */
    Button[i].begin(pin, trig, i); /* Using button */
}

template <uint8_t N>
void LightButton<N>::define_light(uint8_t i, uint8_t pin, bool outputON, bool isPWM, bool isDim, uint8_t defPWM, uint8_t max_pcount, uint8_t limitPWM, int PWMr, uint8_t indicPin)
{
    light[i].Pin = pin;
    light[i].isON = outputON;
    light[i].useDim = isDim;
    light[i].PWMmode = isPWM;

    light[i].auxPin = indicPin;
    light[i].defStep = defPWM;
    light[i].maxSteps = max_pcount;
    light[i].limitPWM = limitPWM;
    light[i].PWMres = PWMr;

    if (!isPWM)
    {
        light[i].init(pin, outputON); /* GPIO output */
    }
    else
    {
        light[i].init(pin, PWMr, isDim); /* PWM output */
    }
    light[i].auxFlag(indicPin); /* init if pin != 255 */
}

template <uint8_t N>
void LightButton<N>::begin()
{
    // _init_onAtBoot();
}

template <uint8_t N>
void LightButton<N>::_newActivity_handler(uint8_t i)
{
    if (OPstr->state) /* ON */
    {
        if (light[i].isPWM())
        {
            if (Button[i].pressCounter <= light[i].maxSteps)
            {
                _turnONlights(i);
                Serial.println("PWM CHNGE");
            }
            else
            {
                Button[N].pressCounter = 0;
                _turnOFFlights(i);
                Button[N].stopTimeout_cb(BUTTON);
            }
        }
        else
        {
            Serial.println("LIGHTS_ON");
            _turnONlights(i);
        }
    }
    else
    {
        Serial.println("LIGHTS_OFF");
        _turnOFFlights(i);
    }
    Button[i].print_OPERstring(*OPstr);

    sendMSG(*OPstr);
    Button[i].newMSG = false;
}

template <uint8_t N>
void LightButton<N>::loop()
{
    for (uint8_t i = 0; i < N; i++)
    {
        Button[i].loop();
        if (Button[i].newMSG)
        {
            _newActivity_handler(i);
        }
    }
}

template <uint8_t N>
unsigned int LightButton<N>::remainClock(uint8_t i)
{
    return Button[i].remainClock();
}

template <uint8_t N>
bool LightButton<N>::getState(uint8_t i)
{
    return Button[i].getState();
}

template <uint8_t N>
void LightButton<N>::stopTimeout_cb(uint8_t reason, uint8_t i)
{
    Button[i].stopTimeout_cb(reason);
}

template <uint8_t N>
void LightButton<N>::startTimeout_cb(int _TO, uint8_t reason, uint8_t i)
{
    Button[i].startTimeout_cb(conv2Minute(_TO), reason);
}

// template <uint8_t N>
// void LightButton<N>::_init_onAtBoot()
// {
//     if (OnatBoot)
//     {
//         startTimeout_cb(0, PWRON);
//     }
// }

template <uint8_t N>
void LightButton<N>::_turnONlights(uint8_t i)
{
    light[i].turnON(Button[i].pressCounter);
}

template <uint8_t N>
void LightButton<N>::_turnOFFlights(uint8_t i)
{
    if (light[i].is_ON())
    {
        light[i].turnOFF();
    }
}

#endif
