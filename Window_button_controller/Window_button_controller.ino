#include <myIOT2.h>
#include <Arduino.h>

#include "myIOT_settings.h"

#define buttonPin D3
#define BUT_PRESSED LOW

void butcmds(byte i)
{
        char *Topic = "myHome/Windows";

        if (i == 1)
        {
                iot.pub_noTopic("up", Topic);
        }
        else if (i == 2)
        {
                iot.pub_noTopic("down", Topic);
        }
        else
        {
                iot.pub_noTopic("off", Topic);
        }
        Serial.println(i);
}
void readButton_lopper()
{
        const byte tstartCMD = 130; // millis
        const byte tbpress = 100;   // millis
        const byte ttoff = 200;     // millis
        bool current_but = digitalRead(buttonPin);
        static bool allow_enter_loop = true;
        static bool lastState = !BUT_PRESSED;
        static unsigned long last_press_clk = 0;
        static byte press_counter = 0;

        if (current_but == BUT_PRESSED && allow_enter_loop == true)
        {
                delay(50);
                if (digitalRead(buttonPin) == BUT_PRESSED)
                {
                        if (press_counter > 0 && millis() - last_press_clk < tbpress * 10) // multi-press detected
                        {
                                press_counter++;
                                Serial.println("A");
                        }

                        else if ((millis() - last_press_clk > tbpress * 10) || (press_counter == 0)) // first press or beyond multipress
                        {
                                press_counter = 1;
                                Serial.println("B");
                        }

                        last_press_clk = millis();
                        while ((digitalRead(buttonPin) == BUT_PRESSED) && (millis() - last_press_clk < ttoff * 10)) //long press
                        {
                                delay(10);
                        }
                        if (millis() - last_press_clk > ttoff * 10)
                        {
                                butcmds(2);
                                press_counter = 0;
                                Serial.println("D");
                                allow_enter_loop = false;
                        }
                }
        }
        else if (current_but != BUT_PRESSED)
        {
                allow_enter_loop = true;
        }

        if (press_counter > 0)
        {
                if (millis() - last_press_clk > 10 * tstartCMD)
                {
                        butcmds(press_counter);
                        press_counter = 0;
                        Serial.println("EXEC");
                        allow_enter_loop = false;
                }
        }
}

void setup()
{
        startIOTservices();
        Serial.println("BOOT!!!!");
        pinMode(buttonPin, INPUT_PULLUP);
}
void loop()
{
        iot.looper();
        readButton_lopper();
        delay(100);
}
