#include <myIOT2.h>
#include <Arduino.h>
#include "myIOT_settings.h"

uint8 BUTPIN_UP = D1;
uint8 LIGHTPIN_UP = D2;
uint8 BUTPIN_DOWN = D3;
uint8 LIGHTPIN_DOWN = D4;

uint8 *inputPins[] = {&BUTPIN_UP, &BUTPIN_DOWN};
uint8 *outputPins[] = {&LIGHTPIN_UP, &LIGHTPIN_DOWN};

#define BUT_PRESSED LOW
#define LIGHT_ON HIGH

class buttonPress
{
#define BUT_PRESSED LOW

public:
        int TIMEOUT_LONG_PRESS = 1500;  /* Above this value press will end and consider long press*/
        int SHORT_PRESS_DEF = 700;      /* Below this valus, it is short press, and above - it is long press */
        uint8 END_LONG_PRESS_VAL = 100; /* Value get spit out to indicate a long press */
        int END_PRESS_DEF = 1000;       /* Max time to wait to next press. after this time when no press occur, send presses */
        uint8 debounce = 50;

private:
        uint8 _pin0;
        uint8 _pin1;
        uint8 _buttonType = 0;
        uint8 _pCounter = 0;
        bool _nowPressed = false;
        bool _statePin0 = false;
        bool _statePin1 = false;
        unsigned long _lastPress = 0;

public:
        buttonPress(uint8 pin0, uint8 _type = 0, uint8 pin1 = 255)
        {
                /* 0: button
                   1: switch
                   2: rocker 3 state
                   3: multiPress inc. long press
                   */
                _pin0 = pin0;
                _pin1 = pin1;
                _buttonType = _type;
                pinMode(_pin0, INPUT_PULLUP);
                if (pin1 != 255)
                {
                        pinMode(_pin1, INPUT_PULLUP);
                }
        }
        uint8 getValue()
        {
                if (_buttonType == 0)
                {
                        return _read_button();
                }
                else if (_buttonType == 1)
                {
                        return _read_switch(_pin0, _statePin0);
                }
                else if (_buttonType == 2) /* 3 state Rocker switch */
                {
                        return _read_rocker();
                }
                else if (_buttonType == 3) /* Short & Long Press*/
                {
                        return _read_multiPress();
                }
                else
                {
                        return 99;
                }
        }

private:
        uint8 _read_button()
        {
                uint8 a = _readPin(_pin0);

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
        uint8 _read_switch(uint8 _pin, bool &_state)
        {
                uint8 a = _readPin(_pin);
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
        uint8 _read_rocker()
        {
                uint8 a = _read_switch(_pin0, _statePin0);
                uint8 b = _read_switch(_pin1, _statePin1);
                if (a == 2 || b == 2)       /* One set to off */
                {
                        return 3;
                }
                else if (a == 1 && b != 1)  /* first in ON while other is not */
                {
                        return 1;
                }
                else if (b == 1 && a != 1)  /* Second in ON while other is not */
                {
                        return 2;
                }
                else                        /* Any other state */
                {
                        return 0;
                }
        }
 
        uint8 _read_multiPress()
        {
                bool a = _readPin(_pin0);

                if (a == 1 && (_nowPressed == false))
                {
                        unsigned long lastRead = millis();
                        _nowPressed = true;

                        while (digitalRead(_pin0) == BUT_PRESSED && millis() - lastRead < TIMEOUT_LONG_PRESS)
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
        uint8 _readPin(uint8 &_pin)
        {
                // PRESSED == 1;
                // NOT PRESSED == 0
                // Err == 2
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
};

void butcmds(uint8 i)
{
        char *Topic = "myHome/Windows";

        if (i == 1)
        {
                blink_lights(i, 2);
                iot.pub_noTopic("up", Topic);
                iot.pub_msg("All-Windows: Set [Up]");
        }
        else if (i == 2)
        {
                blink_lights(i, 2);
                iot.pub_noTopic("down", Topic);
                iot.pub_msg("All-Windows: Set [DOWN]");
        }
        else
        {
                blink_lights(1, 4);
                blink_lights(2, 4);
                iot.pub_noTopic("off", Topic);
                iot.pub_msg("All-Windows: Set [OFF]");
        }
        // delay(2000);
        // Serial.println(i);
}

buttonPress buttonUP(D1, 3);
buttonPress buttonDOWN(D3, 3);

void readButtons()
{
        /* 1 - short press
           100 - long press
           0 - button released
        */
        uint8 upButton_readVal = buttonUP.getValue();
        uint8 downButton_readVal = buttonDOWN.getValue();
        const uint8 sec_part_cmd = 3;
        // Serial.print("state: ");
        // Serial.println(upButton_readVal);

        if (upButton_readVal == 100 || downButton_readVal == 100)
        {
                butcmds(0);
        }
        else if (upButton_readVal == 1)
        {
                butcmds(1);
        }
        else if (downButton_readVal == 1)
        {
                butcmds(2);
        }
        else if (upButton_readVal == 2)
        {
                butcmds(1);
                delay(sec_part_cmd * 1000);
                butcmds(0);
        }
        else if (downButton_readVal == 2)
        {
                butcmds(2);
                delay(sec_part_cmd * 1000);
                butcmds(0);
        }
}
void steady_blink()
{
        static unsigned long lastAction = millis();
        const int offTime = 2000;
        const int onTime = 100;
        if (millis() - lastAction > onTime + offTime)
        {
                digitalWrite(LIGHTPIN_UP, !LIGHT_ON);
                digitalWrite(LIGHTPIN_DOWN, !LIGHT_ON);
                lastAction = millis();
        }
        else if (millis() - lastAction > offTime)
        {
                digitalWrite(LIGHTPIN_UP, LIGHT_ON);
                digitalWrite(LIGHTPIN_DOWN, LIGHT_ON);
        }
}
void blink_lights(uint8 i, uint8 x)
{
        const uint8 DELAY = 100;
        for (int a = 0; a < x; a++)
        {
                digitalWrite(*outputPins[i - 1], !LIGHT_ON);
                digitalWrite(*outputPins[i - 1], LIGHT_ON);
                delay(DELAY);
                digitalWrite(*outputPins[i - 1], !LIGHT_ON);
                delay(DELAY);
        }
}
void setup()
{
        startIOTservices();
        pinMode(LIGHTPIN_UP, OUTPUT);
        pinMode(LIGHTPIN_DOWN, OUTPUT);
}
void loop()
{
        iot.looper();
        readButtons(); /* Delay is already here */
        // steady_blink();
        delay(200);
}
