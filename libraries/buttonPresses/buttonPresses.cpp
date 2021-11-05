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
}
void buttonPresses::start()
{
    pinMode(pin0, INPUT_PULLUP);
    _readPin(pin0, _swState0);
    if (pin1 != 255)
    {
        pinMode(pin1, INPUT_PULLUP);
        _readPin(pin1, _swState1);
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
        return _read_switch(pin0, _swState0, _lastState_pin0);
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
    uint8_t a = _readPin(pin0, _swState0);

    if (a == 1 && _nowPressed == false) /* Press (short or long) */
    {
        _nowPressed = true;
        return 1;
    }
    else if (a == 0 && _nowPressed == true) /*Release from Press */
    {
        _nowPressed = false;
        return 0;
    }
    else /* No change or Err - if holding button a==2*/
    {
        return 0;
    }
}
uint8_t buttonPresses::_read_switch(uint8_t _pin, bool &_state, bool &_pinState)
{
    uint8_t a = _readPin(_pin, _pinState);

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
    uint8_t a = _read_switch(pin0, _swState0, _lastState_pin0);
    uint8_t b = _read_switch(pin1, _swState1, _lastState_pin1);

    if (a == 2 && b == 2) /* Both are off */
    {
        return 0;
    }
    else if (a == 2 || b == 2) /* One set to off */
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
    if (_read_button()) /* Pressed */
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
uint8_t buttonPresses::_readPin(uint8_t &_pin, bool &_state)
{
    // PRESSED == 1; NOT PRESSED == 0 Err == 2
    bool curRead = digitalRead(_pin);
    if (curRead != _state)
    {
        delay(debounce);
        bool curRead2 = digitalRead(_pin);

        if (curRead == curRead2)
        {
            _state = curRead;
            if (curRead == BUT_PRESSED)
            {
                return 1; /* Pressed */
            }
            else
            {
                return 0; /* Released */
            }
        }
        else
        {
            return 2;
        }
    }
    else
    {
        return 2;
    }
}
