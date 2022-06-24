#include <Arduino.h>
#include "buttonPresses.h"

buttonPresses::buttonPresses()
{
}
buttonPresses::buttonPresses(uint8_t _pin0, uint8_t _type, uint8_t _pin1)
{
    /* 0: button
       1: switch
       2: rocker 3 state
       3: multiPress inc. long press
    */
    pin0 = _pin0;
    pin1 = _pin1;
    buttonType = _type;
}
void buttonPresses::start()
{
    pinMode(pin0, INPUT_PULLUP); /* INPUT_PULLUP*/ 
    _lastState_pin0 = digitalRead(pin0);
    if (pin1 != 255)
    {
        pinMode(pin1, INPUT_PULLUP);
        _lastState_pin1 = digitalRead(pin1);
    }
}
uint8_t buttonPresses::read()
{
    if (buttonType < 3)
    {
        return _readPin();
    }
    else if (buttonType == 3) /* Short & Long Press*/
    {
        return _read_multiPress();
    }
    else
    {
        return 99;
    }
}

uint8_t buttonPresses::_read_multiPress()
{
    if (_readPin() == 1)
    {
        /* calc press duration */
        unsigned long current_press_duration = millis();
        while (digitalRead(pin0) == BUT_PRESSED && millis() - current_press_duration < TIMEOUT_LONG_PRESS)
        {
            delay(10);
        }
        int press_duration = millis() - current_press_duration;

        /* Was it a short press or a long press ? */
        if (press_duration < SHORT_PRESS_DEF)
        {
            _pCounter++;
        }
        else
        {
            _lastPress = millis();
            return END_LONG_PRESS_VAL; /* Case of long press - return long press value */
        }
        _lastPress = millis();
    }

    if (_pCounter > 0 && millis() - _lastPress > END_PRESS_DEF) /* After timeout - return pCounts */
    {
        byte a = _pCounter;
        _pCounter = 0;
        _lastPress = 0;
        return a;
    }
    else /* Return 0 if no change */
    {
        return 0;
    }
}
uint8_t buttonPresses::_readPin()
{
    bool _curReadPin0_1 = digitalRead(pin0);
    bool _curReadPin1_1 = digitalRead(pin1);

    if (_curReadPin0_1 != _lastState_pin0 || _curReadPin1_1 != _lastState_pin1)
    {
        delay(debounce);
        bool _curReadPin0_2 = digitalRead(pin0);
        bool _curReadPin1_2 = digitalRead(pin1);

        if (_curReadPin0_1 == _curReadPin0_2 && _curReadPin0_1 != _lastState_pin0) /* Change in pin0 */
        {
            _lastState_pin0 = _curReadPin0_1;
            // return _lastState_pin0;
            if (_curReadPin0_1 == BUT_PRESSED)
            {
                return 1; /* Pressed */
            }
            else
            {
                return 0; /* Released */
            }
        }
        else if (pin1 != 255 && _curReadPin1_1 == _curReadPin1_2 && _curReadPin1_1 != _lastState_pin1)
        {
            _lastState_pin1 = _curReadPin1_1;
            if (_curReadPin1_1 == BUT_PRESSED)
            {
                return 2; /* Pressed */
            }
            else
            {
                return 0; /* Released */
            }
        }
        else
        {
            return 3; /* Err*/
        }
    }
    else
    {
        return 4; /* No change*/
    }
}
