#include <myIOT.h>
#include "myIOT_settings.h"
#include <Arduino.h>

const int sensPins[] = {D3, D5};
void startGPIO()
{
        for (int i = 0; i < sizeof(sensPins) / sizeof(sensPins[0]); i++)
        {
                pinMode(sensPins[i], INPUT);
        }
}
bool readSignal(const int pins[], int s_arr)
{
        bool result = true;
        for (int i = 0; i < s_arr; i++)
        {
                result *= digitalRead(pins[i]);
        }
        return result;
}
bool scanRadar(const int reTrigger = 180, const int verifyDetectPeriod = 5, const int remainTriggered = 3)
{
        static bool lastState = false;
        static bool det_notif = false;
        static unsigned long last_det_clock = 0;
        static unsigned long det_clock = 0;
        static int det_counter = 0;

        bool state = readSignal(sensPins, sizeof(sensPins) / sizeof(sensPins[0]));
        if (state && det_notif == false)
        {
                if (det_clock == 0 && millis() > 1000UL * reTrigger + last_det_clock)
                {
                        /* first indication for detection */
                        det_clock = millis();
                        return 0;
                }
                else if (det_clock > 0 && millis() - det_clock > 1000UL * verifyDetectPeriod)
                {
                        /* Now it is a certified detection */
                        det_notif = true;
                        det_clock = millis();
                        return 1;
                }
                else
                {
                        return 0;
                }
        }
        else if (state == false && det_clock > 0 && det_notif == false)
        {
                /* Clearing flags after a non-certified detection */
                det_clock = 0;
                return 0;
        }
        else if (state == false && det_notif && millis() > remainTriggered * 1000UL + det_clock)
        {
                /* Ending detection mode, clearing flags */
                det_notif = false;
                det_clock = 0;
                last_det_clock = millis();
                return 0;
        }
}
void Radar_looper()
{
        static bool inAlert = false;
        bool r = scanRadar(15, 1.5);
        if (r && inAlert == false)
        {
                inAlert = true;
                iot.pub_msg("Detection");
        }
        else if (r == false && inAlert == true)
        {
                inAlert = false;
        }
}

void setup()
{
        startIOTservices();
        startGPIO();
}
void loop()
{
        iot.looper();
        Radar_looper();
        delay(100);
}
