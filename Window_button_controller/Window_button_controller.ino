#include <myIOT2.h>
#include <Arduino.h>

#include "myIOT_settings.h"

byte BUTPIN_UP = D1;
byte LIGHTPIN_UP = D2;
byte BUTPIN_DOWN = D3;
byte LIGHTPIN_DOWN = D4;

#define BUT_PRESSED LOW
#define LIGHT_ON HIGH

bool lastUP = !BUT_PRESSED;
bool lastDOWN = !BUT_PRESSED;
class buttonPress
{
#define BUT_PRESSED LOW

public:
        buttonPress(byte pin0, byte pin1 = 255, byte _type = 0)
        {
                /* 0: pushbutton
                   1: toggle
                   2: rocker 3 state 
                   */
                _pin0 = pin0;
                _pin1 = pin1;
                buttonType = _type;
                pinMode(_pin0, INPUT_PULLUP);
                if (pin1 != 255)
                {
                        pinMode(_pin1, INPUT_PULLUP);
                }
        }
        byte read_button()
        {
                bool curRead = digitalRead(_pin0);
                delay(50);
                bool curRead2 = digitalRead(_pin0);

                if (curRead == curRead2 && curRead == BUT_PRESSED && _nowPressed == false)
                {
                        unsigned long lastRead = millis();
                        _nowPressed = true;
                        while (digitalRead(_pin0) == BUT_PRESSED && millis() - lastRead < 2000)
                        {
                                delay(10);
                        }
                        int calc = millis() - lastRead;
                        if (calc < 500)
                        {
                                return 1;
                        }
                        else
                        {
                                return 2;
                        }
                }
                else
                {
                        _nowPressed = false;
                        return 0;
                }
        }
        byte read_toggle(byte _pin, bool &_state)
        {
                bool curRead = digitalRead(_pin);
                delay(50);
                bool curRead2 = digitalRead(_pin);

                if (curRead == curRead2 && curRead != _state)
                {
                        _state = curRead;
                        if (curRead == !BUT_PRESSED)
                        {
                                return 0; // OFF
                        }
                        else
                        {
                                if (_pin == _pin0)
                                {
                                        return 1;
                                }
                                else if (_pin == _pin1)
                                {
                                        return 2;
                                }
                        }
                }
        }
        byte read_pushbutton()
        {
                bool curRead = digitalRead(_pin0);
                delay(50);
                bool curRead2 = digitalRead(_pin0);

                if (curRead == curRead2 && curRead == BUT_PRESSED)
                {
                        return 1;
                }
                else
                {
                        return 0;
                }
        }
        void loop()
        {
                if (buttonType == 0)
                {
                        read_button();
                }
                else if (buttonType == 1)
                {
                        read_toggle(_pin0, _state);
                        read_toggle(_pin1, _stateAux);
                }
        }

private:
        byte _pin0;
        byte _pin1;
        byte buttonType = 0;
        bool _nowPressed = false;
        bool _state = false;
        bool _stateAux = false;
};

void butcmds(byte i)
{
        char *Topic = "myHome/Windows";

        if (i == 1)
        {
                // iot.pub_noTopic("up", Topic);
                // iot.pub_msg("All-Windows: Set [Up]");
                digitalWrite(LIGHTPIN_UP, LIGHT_ON);
        }
        else if (i == 2)
        {
                // iot.pub_noTopic("down", Topic);
                // iot.pub_msg("All-Windows: Set [DOWN]");
                digitalWrite(LIGHTPIN_DOWN, LIGHT_ON);
        }
        else
        {
                // iot.pub_noTopic("off", Topic);
                // iot.pub_msg("All-Windows: Set [OFF]");
                digitalWrite(LIGHTPIN_UP, !LIGHT_ON);
                digitalWrite(LIGHTPIN_DOWN, !LIGHT_ON);
        }
        delay(200);
        Serial.println(i);
}
void read_toggle(byte _pin, bool &_state)
{
        bool curRead = digitalRead(_pin);
        delay(50);
        bool curRead2 = digitalRead(_pin);

        if (curRead == curRead2 && curRead != _state)
        {
                if (curRead == !BUT_PRESSED)
                {
                        butcmds(0); // OFF
                }
                else
                {
                        if (_pin == BUTPIN_UP)
                        {
                                butcmds(1); // UP
                        }
                        else if (_pin == BUTPIN_DOWN)
                        {
                                butcmds(2); // DOWN
                        }
                }
                _state = curRead;
        }
}
void read_button(byte _pin)
{
        bool curRead = digitalRead(_pin);
        delay(50);
        bool curRead2 = digitalRead(_pin);

        if (curRead == curRead2 && curRead == BUT_PRESSED)
        {
                unsigned long lastRead = millis();
                while (digitalRead(_pin) == BUT_PRESSED && millis() - lastRead < 2000)
                {
                        delay(10);
                }
                int calc = millis() - lastRead;
                if (calc < 500)
                {
                        if (_pin == BUTPIN_UP)
                        {
                                butcmds(1); // UP
                        }
                        else if (_pin == BUTPIN_DOWN)
                        {
                                butcmds(2); // DOWN
                        }
                }
                else
                {
                        butcmds(0); // OFF
                }
        }
}
void setup()
{
        startIOTservices();
        // pinMode(BUTPIN_UP, INPUT_PULLUP);
        // pinMode(BUTPIN_DOWN, INPUT_PULLUP);
        pinMode(LIGHTPIN_UP, OUTPUT);
        pinMode(LIGHTPIN_DOWN, OUTPUT);
}
void loop()
{
        iot.looper();
        // read_toggle(BUTPIN_UP, lastUP);
        // read_toggle(BUTPIN_DOWN, lastDOWN);
        read_button(BUTPIN_UP);
        read_button(BUTPIN_DOWN);
        delay(100);
}
