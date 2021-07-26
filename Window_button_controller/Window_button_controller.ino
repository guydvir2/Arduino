#include <myIOT2.h>
#include <Arduino.h>

#include "myIOT_settings.h"

byte BUTPIN_UP = D1;
byte BUTPIN_DOWN = D2;

#define BUT_PRESSED LOW
bool lastUP = !BUT_PRESSED;
bool lastDOWN = !BUT_PRESSED;

void butcmds(byte i)
{
        char *Topic = "myHome/Windows";

        if (i == 1)
        {
                iot.pub_noTopic("up", Topic);
                iot.pub_msg("All-Windows: Set [Up]");
        }
        else if (i == 2)
        {
                iot.pub_noTopic("down", Topic);
                iot.pub_msg("All-Windows: Set [DOWN]");
        }
        else
        {
                iot.pub_noTopic("off", Topic);
                iot.pub_msg("All-Windows: Set [OFF]");
        }
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
void setup()
{
        startIOTservices();
        pinMode(BUTPIN_UP, INPUT_PULLUP);
        pinMode(BUTPIN_DOWN, INPUT_PULLUP);
}
void loop()
{
        iot.looper();
        read_toggle(BUTPIN_UP, lastUP);
        read_toggle(BUTPIN_DOWN, lastDOWN);
        delay(100);
}
