#ifndef but_presses_h
#define but_presses_h
#include "Arduino.h"

class buttonPresses
{
    /* 0: button
       1: switch
       2: rocker 3 state
       3: multiPress inc. long press
    */
#define BUT_PRESSED LOW

public:
    int TIMEOUT_LONG_PRESS = 1000;    /* Above this value press will end and consider long press*/
    int SHORT_PRESS_DEF = 700;        /* Below this valus, it is short press, and above - it is long press */
    int END_PRESS_DEF = 1000;         /* Max time to wait to next press. after this time when no press occur, send presses */
    uint8_t END_LONG_PRESS_VAL = 100; /* Value get spit out to indicate a long press */
    uint8_t debounce = 50;            /* millis debounce between 2 io reading */
    uint8_t buttonType = 0;
    uint8_t pin0 = 0;
    uint8_t pin1 = 255;

private:
    uint8_t _pCounter = 0;
    bool _lastState_pin0 = false;
    bool _lastState_pin1 = false;
    unsigned long _lastPress = 0;

public:
    buttonPresses(uint8_t _pin0, uint8_t _type = 0, uint8_t _pin1 = 255);
    buttonPresses();
    void start();
    uint8_t read();

private:
    uint8_t _read_multiPress();
    uint8_t _readPin();
};

#endif