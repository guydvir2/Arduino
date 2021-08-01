#include "Arduino.h"
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
    // start();
}
void buttonPresses::start()
{
    pinMode(pin0, INPUT_PULLUP);
    if (pin1 != 255)
    {
        pinMode(pin1, INPUT_PULLUP);
    }
}
uint8_t buttonPresses::getValue()
{
    if (buttonType == 0)
    {
        return _read_button();
    }
    else if (buttonType == 1)
    {
        return _read_switch(pin0, _statePin0);
    }
    else if (buttonType == 2) /* 3 state Rocker switch */
    {
        return _read_rocker();
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
uint8_t buttonPresses::_read_button()
{
    uint8_t a = _readPin(pin0);

    if (a == 1 && _nowPressed == false) /* Press */
    {
        _nowPressed = true;
        return 1;
    }
    else if (a == 0 && _nowPressed == true) /*Release from Press */
    {
        _nowPressed = false;
        return 0;
    }
    else /* No change or Err */
    {
        return 0;
    }
}
uint8_t buttonPresses::_read_switch(uint8_t _pin, bool &_state)
{
    uint8_t a = _readPin(_pin);
    if (a != 2 && (a != _state))
    {
        _state = a;
        if (a == 1)
        {
            return 1; /* ON state*/
        }
        else
        {
            return 2; /* OFF state*/
        }
    }
    else
    {
        return 0; /* Err or no change */
    }
}
uint8_t buttonPresses::_read_rocker()
{
    uint8_t a = _read_switch(pin0, _statePin0);
    uint8_t b = _read_switch(pin1, _statePin1);
    if (a == 2 || b == 2) /* One set to off */
    {
        return 3;
    }
    else if (a == 1 && b != 1) /* first in ON while other is not */
    {
        return 1;
    }
    else if (b == 1 && a != 1) /* Second in ON while other is not */
    {
        return 2;
    }
    else /* Any other state */
    {
        return 0;
    }
}
uint8_t buttonPresses::_read_multiPress()
{
    bool a = _readPin(pin0);

    if (a == 1 && (_nowPressed == false))
    {
        unsigned long lastRead = millis();
        _nowPressed = true;

        while (digitalRead(pin0) == BUT_PRESSED && millis() - lastRead < TIMEOUT_LONG_PRESS)
        {
            delay(10);
        }
        int calc = millis() - lastRead;
        if (calc < SHORT_PRESS_DEF)
        {
            _pCounter++;
        }
        else
        {
            _lastPress = millis();
            return END_LONG_PRESS_VAL;
        }
        _lastPress = millis();
    }
    else if (a == 0 && (_nowPressed == true))
    {
        _nowPressed = false;
    }

    if (_pCounter > 0 && millis() - _lastPress > END_PRESS_DEF)
    {
        byte a = _pCounter;
        _pCounter = 0;
        _lastPress = 0;
        return a;
    }
    else
    {
        return 0;
    }
}
uint8_t buttonPresses::_readPin(uint8_t &_pin)
{
    // PRESSED == 1; NOT PRESSED == 0 Err == 2
    bool curRead = digitalRead(_pin);
    delay(debounce);
    bool curRead2 = digitalRead(_pin);
    if (curRead == curRead2)
    {
        if (curRead == BUT_PRESSED)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 2;
    }
}
