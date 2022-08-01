#ifndef TOB_h
#define TOB_h

#include <Arduino.h>
#include <Button2.h>
#include <Chrono.h>
#include <TurnOnLights.h>

class timeoutButton
{
private:
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

    bool _useInput = false;

private:
    Chrono chrono;
    Button2 button;

public:
    TurnOnLights *Light;

public:
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

    void set_lights(TurnOnLights *light);
    virtual bool lightsON(uint8_t step = 1);
    virtual bool lightsOFF();
    virtual bool lightsPWM(int val);

    void loop();
    bool getState();
    unsigned int remainWatch();

    bool OFF_cb(uint8_t reason);
    bool ON_cb(int _TO, uint8_t reason);
};

class LightButton
{
private:
    uint8_t _buttonID = 0;
    TurnOnLights Light;
    timeoutButton Button;

public:
    bool OnatBoot = false;
    bool dimmablePWM = true;

    bool output_ON = HIGH;
    bool inputPressed = LOW;

    uint8_t trigType = 0;
    uint8_t inputPin = 5;
    uint8_t outputPin = 4;
    uint8_t indicPin = 255;

    int def_TO_minutes = 360;
    int maxON_minutes = 1000;

    uint8_t defPWM = 2;
    uint8_t max_pCount = 3;
    uint8_t limitPWM = 50;
    int PWM_res = 1023;

    LightButton();
    // ~~~~~~~~ Belongs to Button Class ~~~~~
    void loop();
    bool getState();
    void begin(uint8_t id);
    unsigned int remainWatch();
    void OFF_cb(uint8_t reason);
    void ON_cb(int _TO, uint8_t reason);
    
    // ~~~~~~~~ Belongs to Light Class ~~~~~
    bool isON();
    void blink(uint8_t blinks, int _delay = 20);
};
#endif
