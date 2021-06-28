#include <myIOT2.h>
#include <Arduino.h>

#define OUTPUT_PIN D1
#define OUTPUT_ON HIGH
#define INPUT_PIN D3
#define INPUT_PRESSED LOW

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"
#include <myTimeoutSwitch.h>

timeOUTSwitch timeoutSW_0;

void startIO()
{
        pinMode(OUTPUT_PIN, OUTPUT);
        digitalWrite(OUTPUT_PIN, !OUTPUT_ON);
}
bool get_SWstate()
{
        if (digitalRead(OUTPUT_PIN) == OUTPUT_ON)
        {
                return 1;
        }
        else
        {
                return 0;
        }
}
bool switchIt(bool state)
{
        if (state != get_SWstate())
        {
                if (state == HIGH)
                {
                        digitalWrite(OUTPUT_PIN, OUTPUT_ON);
                }
                else
                {
                        digitalWrite(OUTPUT_PIN, !OUTPUT_ON);
                }
                return 1;
        }
        else
        {
                return 0;
        }
}
void simplifyClock(char *days, char *clk, char retVal[25])
{
        if (strcmp(days, "0d") != 0)
        {
                sprintf(retVal, "%s %s", days, clk);
        }
        else
        {
                sprintf(retVal, "%s", clk);
        }
}
void sf(char msg1[50])
{
        char msg[100];
        char s1[25];
        char s2[7];
        char clk[25];
        Serial.println("START");
        if (get_SWstate() == 0)
        {
                iot.convert_epoch2clock(timeoutSW_0.TO_duration_sec, 0, s1, s2);
                simplifyClock(s2, s1, clk);
                switchIt(HIGH);
                sprintf(msg, "%s: Switched [ON] for [%s]", msg1, clk);
                iot.pub_msg(msg);
        }
        else
        {
                Serial.println("ONError");
                switchIt(LOW);
        }
}
void ef(char msg1[50])
{
        char msg[100];
        char s1[15];
        char s2[7];
        char clk[25];
        char clk2[25];
        Serial.println("END");
        if (get_SWstate() == 1)
        {
                switchIt(LOW);
                if (timeoutSW_0.remTime() > 0)
                {
                        iot.convert_epoch2clock(timeoutSW_0.TO_duration_sec - timeoutSW_0.remTime(), 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        iot.convert_epoch2clock(timeoutSW_0.remTime(), 0, s1, s2);
                        simplifyClock(s2, s1, clk2);
                        sprintf(msg, "%s: Switched [OFF] after [%s], remain [%s]", msg1, clk, clk2);
                }
                else
                {
                        iot.convert_epoch2clock(timeoutSW_0.TO_duration_sec, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        sprintf(msg, "%s: Switched [OFF] ended after [%s]", msg1, clk);
                }
        }
        else
        {
                Serial.println("END Error");
                switchIt(0);
        }
        iot.pub_msg(msg);
}
void start_timeOUT()
{
        timeoutSW_0.maxON_minutes = 30;
        timeoutSW_0.trigType = 0;
        timeoutSW_0.startIO(INPUT_PIN, INPUT_PRESSED);
        timeoutSW_0.def_funcs(sf, ef);
}
void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
#endif
        startIO();
        start_timeOUT();
}
void loop()
{
        iot.looper();
        timeoutSW_0.looper();
        delay(100);
}
