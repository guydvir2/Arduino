#include <myIOT2.h>
// #include "empty_param.h"
#include "myIOT_settings.h"
#include <Arduino.h>

int PIRpin = D7;
const bool DetectioState = LOW;
bool PIRlooper()
{
        static bool lastState = !DetectioState;
        bool currentState = digitalRead(PIRpin);
        if (lastState == !DetectioState && currentState == DetectioState)
        {
                lastState = DetectioState;
                return 1;
        }
        else if (lastState == DetectioState && currentState == !DetectioState)
        {
                lastState = !DetectioState;
                return 0;
        }
        else
        {
                return 0;
        }
}

void setup()
{
        startIOTservices();
        pinMode(PIRpin, INPUT_PULLUP);
}
void loop()
{
        iot.looper();
        if (PIRlooper())
        {
                iot.pub_msg("Detection");
        }
        delay(100);
}
