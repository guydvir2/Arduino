#include <myIOT2.h>
#include <Arduino.h>
#include <buttonPresses.h>
#include "myIOT_settings.h"

#define BUT_PRESSED LOW
#define LIGHT_ON HIGH

uint8_t BUTPIN_UP = D1;
uint8_t LIGHTPIN_UP = D2;
uint8_t BUTPIN_DOWN = D3;
uint8_t LIGHTPIN_DOWN = D4;
uint8_t *outputPins[] = {&LIGHTPIN_UP, &LIGHTPIN_DOWN};

buttonPresses buttUP(BUTPIN_UP, 3);
buttonPresses buttDOWN(BUTPIN_DOWN, 3);

bool blinkNow = false;

void butcmds(uint8_t i)
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
}
void readButtons()
{
        /* 1 - short press
           100 - long press
           0 - button released
        */

        uint8_t upButton_readVal = buttUP.getValue();
        uint8_t downButton_readVal = buttDOWN.getValue();
        const uint8_t sec_part_cmd = 3;

        if (upButton_readVal == 100 || downButton_readVal == 100)
        {
                butcmds(0); // Off
        }
        else if (upButton_readVal == 1)
        {
                butcmds(1); // UP
        }
        else if (downButton_readVal == 1)
        {
                butcmds(2); // Down
        }
        // else if (upButton_readVal == 2)
        // {
        //         butcmds(1);
        //         delay(sec_part_cmd * 1000);
        //         butcmds(0);
        // }
        // else if (downButton_readVal == 2)
        // {
        //         butcmds(2);
        //         delay(sec_part_cmd * 1000);
        //         butcmds(0);
        // }
}
void steady_blink()
{
        static unsigned long lastAction = millis();
        const int offTime = 2000;
        const int onTime = 150;
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
void blink_lights(uint8_t i, uint8_t x)
{
        if (blinkNow)
        {
                const uint8_t DELAY = 100;
                // for (int a = 0; a < x; a++)
                // {
                //         digitalWrite(*outputPins[i - 1], !LIGHT_ON);
                //         digitalWrite(*outputPins[i - 1], LIGHT_ON);
                //         delay(DELAY);
                //         digitalWrite(*outputPins[i - 1], !LIGHT_ON);
                //         delay(DELAY);
                // }
        }
}
void setup()
{
        buttDOWN.start();
        buttUP.start();
        pinMode(LIGHTPIN_UP, OUTPUT);
        pinMode(LIGHTPIN_DOWN, OUTPUT);
        startIOTservices();
}
void loop()
{
        iot.looper();
        readButtons(); /* Delay is already here */
        // steady_blink();
        delay(50);
}