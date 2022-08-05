#ifndef turnonlights_h
#define turnonlights_h

#include <Arduino.h>

class TurnOnLights
{
#define TURN_ON digitalWrite(Pin, isON)
#define TURN_OFF digitalWrite(Pin, !isON)
#define INDIC_ON _useAuxFlag ? digitalWrite(auxPin, HIGH) : yield()
#define INDIC_OFF _useAuxFlag ? digitalWrite(auxPin, LOW) : yield()
#define IS_OUTPUT_ON digitalRead(Pin) == isON

private:
    bool _useAuxFlag = false; /* PWM & GPIO */

public:
    int PWMval = 0;
    int PWMres = 1023;

    bool isON = HIGH;    /* GPIO only */
    bool useDim = false; /* PWM only */
    bool PWMmode = true; /* select PWM or GPIO */

    uint8_t Pin = 255;
    uint8_t auxPin = 255;
    uint8_t defStep = 2;
    uint8_t dimDelay = 1;
    uint8_t maxSteps = 3;
    uint8_t limitPWM = 100; /* Percentage of total power */
    uint8_t currentStep = 0;

protected:
    const char *ver = "turnLight_v0.2";

public:
    TurnOnLights();
    void auxFlag(uint8_t pin = 255);
    void init(uint8_t pin, bool _isON);            /* GPIO */
    void init(uint8_t pin, int res, bool usedim); /* PWM */

    bool is_ON();
    bool isPWM();
    bool turnOFF();                /* PWM & GPIO */
    bool turnON(uint8_t step = 0); /* PWM & GPIO */
    bool PWMvalue(int val);        /* Drive to PWM value */
    void blink(uint8_t blinks, int _delay = 20);

private:
    bool _setPWM(int val);
    void _Dim2Value(int val);
    bool _isValidPWM(int val);
    bool _isValidStep(int step);
    int _step2Value(uint8_t step);
};

#endif
