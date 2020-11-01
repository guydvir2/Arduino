#include <myIOT.h>
#include "myIOT_settings.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_0.1"

int sensorPin = D1;

void scanRadar()
{
        static bool lastState = false;
        static bool det_notif = false;
        static unsigned long last_det_clock = 0;
        static unsigned long det_clock = 0;
        const int reTrigger = 15;         //seconds to wait until next detection
        const int verifyDetectPeriod = 3; /*seconds to be in detection mode - to avoid quick false alarms*/
        const int remainTriggered = 3;    /*seconds until declaring "end Detection" */
        static int det_counter = 0;

        bool state = digitalRead(sensorPin);
        if (state && det_notif == false)
        {
                if (det_clock == 0 && millis() > 1000UL * reTrigger + last_det_clock)
                {
                        det_clock = millis();
                        Serial.println("pre_detect");
                }
                else if (det_clock >0&& millis() - det_clock > 1000UL * verifyDetectPeriod)
                {
                        Serial.println("Detection");
                        char a[20];
                        sprintf(a, "Detection: detCount[#%d]", det_counter++);
                        iot.pub_msg(a);
                        det_notif = true;
                        det_clock = millis();
                }
        }
        else if (state == false && det_clock>0 && millis() > det_clock + 1000 * verifyDetectPeriod && det_notif == false)
        {
                det_clock = 0;
                Serial.println("clearing false alarm");
        }
        else if (state == false && det_notif && millis() > remainTriggered * 1000UL + det_clock)
        {
                Serial.println("END Detection");
                det_notif = false;
                det_clock = 0;
                last_det_clock = millis();
        }
}
void setup()
{
        startIOTservices();
        pinMode(sensorPin, INPUT);
}
void loop()
{
        iot.looper();
        scanRadar();
        delay(100);
}
