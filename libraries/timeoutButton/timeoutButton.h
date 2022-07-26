#ifndef TOB_h
#define TOB_h

#include <Arduino.h>
#include <Button2.h>
#include <Chrono.h>
#include <TurnOnLights.h>

class timeoutButton
{
private:
    typedef void (*cb_func)(uint8_t resaon, uint8_t i);
#define conv2Minute(t) t * 60

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
        BUTTON
    };

    uint8_t _pin = 255;
    uint8_t _trigType = 0;
    uint8_t _pressCounter = 0;
    unsigned long _lastPress = 0;

private:
    Chrono chrono;
    Button2 button;

    cb_func _extOn_cb = nullptr;
    cb_func _extOff_cb = nullptr;
    cb_func _extMultipress_cb = nullptr;

public:
    bool useInput = false;
    int timeout = 0;
    int maxTimeout = 500;
    int defaultTimeout = 1;
    int time2Repress = 1000; // millis
    uint8_t _id = 0;

protected:
    const char *ver = "timeouter_v0.3";

private:
    void _init_button();
    void _Button_looper();
    void _ON_OFF_on_handle(Button2 &b);
    void _Momentary_handle(Button2 &b);
    void _ON_OFF_off_handle(Button2 &b);
    void _TrigSensor_handler(Button2 &b);
    void _MultiPress_handler(Button2 &b);

    void _stopWatch();
    void _loopWatch();
    void _startWatch();
    void _init_chrono();

public:
    timeoutButton();
    void begin(uint8_t id); /* Not using button */
    void begin(uint8_t pin, uint8_t trigType, uint8_t id = 0);
    void addWatch(int _add, uint8_t reason);
    void loop();
    bool getState();
    unsigned int remainWatch();

    void OFF_cb(uint8_t reason);         // const char *trigger);
    void ON_cb(int _TO, uint8_t reason); //, const char *trigger = nullptr, uint8_t _PWMstep = 1);
    void ExtON_cb(cb_func func);
    void ExtOFF_cb(cb_func func);
    void ExtMultiPress_cb(cb_func func);
};

template <uint8_t N>
class timeout2
{
private:
    timeoutButton _toButton[N];
    TurnOnLights _lightOutput[N];

    void _init_TObutton();
    void _init_Light();
    void _initAll();
    void _onAtBoot();

public:
    bool OnatBoot[N];
    bool useInput[N];
    bool outputPWM[N];
    bool useIndicLED[N];
    bool dimmablePWM[N];

    bool output_ON[N];
    bool inputPressed[N];

    uint8_t trigType[N];
    uint8_t inputPin[N];
    uint8_t outputPin[N];
    uint8_t indicPin[N];

    int def_TO_minutes[N];
    int maxON_minutes[N];

    uint8_t defPWM[N];
    uint8_t max_pCount[N];
    uint8_t limitPWM[N];
    uint8_t PWM_res;

    char sw_names[N][15];

    timeout2();
    void begin();
    void turnON_lightsCB(uint8_t reason);
    void turnOFF_lightsCB(uint8_t reason);
    void MULTP_CB(uint8_t reason);
    void loop();
};

template <uint8_t N>
timeout2<N>::timeout2()
{
}
template <uint8_t N>
void timeout2<N>::begin()
{
    _init_TObutton();
    _init_Light();
}
template <uint8_t N>
void timeout2<N>::turnON_lightsCB(uint8_t reason)
{
}
template <uint8_t N>
void timeout2<N>::turnOFF_lightsCB(uint8_t reason)
{
}
template <uint8_t N>
void timeout2<N>::MULTP_CB(uint8_t reason)
{
}

template <uint8_t N>
void timeout2<N>::_init_Light()
{
    for (uint8_t i = 0; i < N; i++)
    {
        if (outputPWM[i])
        {
            _lightOutput[i].init(outputPin[i], PWM_res, dimmablePWM[i]);
            _lightOutput[i].defStep = defPWM[i];
            _lightOutput[i].maxSteps = max_pCount[i];
            _lightOutput[i].limitPWM = limitPWM[i];
        }
        else
        {
            _lightOutput[i].init(outputPin[i], output_ON[i]);
        }
        if (useIndicLED[i])
        {
            _lightOutput[i].auxFlag(indicPin[i]);
        }
    }
}

template <uint8_t N>
void timeout2<N>::_init_TObutton()
{
    for (uint8_t i = 0; i < N; i++)
    {
        _toButton[i].ExtON_cb(turnON_lightsCB);
        // _toButton[i].ExtOFF_cb(turnOFF_lightsCB);
        // _toButton[i].ExtMultiPress_cb(MULTP_CB);

        if (useInput[i])
        {
            _toButton[i].begin(inputPin[i], trigType[i], i);
        }
        else
        {
            _toButton[i].begin(i);
        }
    }
}

template <uint8_t N>
void timeout2<N>::loop()
{
    for (uint8_t i = 0; i < N; i++)
    {
        _toButton[i].loop();
    }
}

#endif
