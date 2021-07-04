#include <myIOT2.h>
#include <Arduino.h>
#include <myTimeoutSwitch.h>

timeOUTSwitch timeoutSW_0;

/* Values get uodated from parameter files */
bool inputPressed = LOW;
bool output_ON = HIGH;
bool OnatBoot = true;
byte inputPin = 1;
byte outputPin = 2;
byte defPWM = 2;
byte limitPWM = 80;
int PWM_res = 1023;
/* End */

#include "myTO_param.h"
#include "myIOT_settings.h"

void startIO()
{
        pinMode(outputPin, OUTPUT);
        digitalWrite(outputPin, !output_ON);
}
bool get_SWstate()
{
        if (timeoutSW_0.trigType != 3)
        {
                if (digitalRead(outputPin) == output_ON)
                {
                        return 1;
                }
                else
                {
                        return 0;
                }
        }
        else /* PWM */
        {
                if (timeoutSW_0.pwm_pCount > 0)
                {
                        return 1;
                }
                else
                {
                        return 0;
                }
        }
}
bool switchIt(bool state)
{
        if (state != get_SWstate())
        {
                if (state == HIGH)
                {
                        digitalWrite(outputPin, output_ON);
                }
                else
                {
                        digitalWrite(outputPin, !output_ON);
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
int convDim(byte dim_step)
{
        return (int)((dim_step * PWM_res * limitPWM) / (timeoutSW_0.totPWMsteps * 100));
}
void PWMdim(int dim_step)
{
        byte sign = 2;
        byte delay_step = 2;
        byte C_val = 1;
        int desiredval = 0;
        static int _last_dimVal = 0;
        
        desiredval = convDim(dim_step);

        if (desiredval < _last_dimVal)
        {
                C_val = -1 * C_val;
        }
        while (abs(desiredval - _last_dimVal) >= 1)
        {
                _last_dimVal = _last_dimVal + C_val;
                analogWrite(outputPin, _last_dimVal);
                delay(delay_step);
        }
}
void switchON_cb(char msg1[50])
{
        char msg[100];
        char s1[25];
        char s2[7];
        char clk[25];

        if (timeoutSW_0.trigType != 3)
        {
                if (get_SWstate() == 0)
                {
                        iot.convert_epoch2clock(timeoutSW_0.TO_duration_minutes * 60, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        switchIt(HIGH);
                        sprintf(msg, "%s: Switched [ON] for [%s]", msg1, clk);
                        iot.pub_msg(msg);
                }
        }
        else
        {
                if (timeoutSW_0.pwm_pCount == 0)
                {
                        timeoutSW_0.pwm_pCount = 1;
                }
                PWMdim(timeoutSW_0.pwm_pCount);

                iot.convert_epoch2clock(timeoutSW_0.TO_duration_minutes * 60, 0, s1, s2);
                simplifyClock(s2, s1, clk);
                sprintf(msg, "%s: Switched [ON] Power[%d%%] for [%s]", msg1, (int)(100 * timeoutSW_0.pwm_pCount / timeoutSW_0.totPWMsteps), clk);
                iot.pub_msg(msg);
        }
}
void switchOFF_cb(char msg1[50])
{
        char msg[100];
        char s1[15];
        char s2[7];
        char clk[25];
        char clk2[25];

        if (get_SWstate() == 1)
        {
                if (timeoutSW_0.trigType != 3)
                {
                        switchIt(LOW);
                }
                else
                {
                        PWMdim(0);
                }
                int a = (int)((millis() - timeoutSW_0.TO_start_millis) / 1000);
                if (timeoutSW_0.remTime() > 0)
                {
                        iot.convert_epoch2clock(a, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        iot.convert_epoch2clock(timeoutSW_0.remTime(), 0, s1, s2);
                        simplifyClock(s2, s1, clk2);
                        sprintf(msg, "%s: Switched [OFF] after [%s], remained [%s]", msg1, clk, clk2);
                }
                else
                {
                        iot.convert_epoch2clock(a, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        sprintf(msg, "%s: Switched [OFF] ended after [%s]", msg1, clk);
                }
        }
        iot.pub_msg(msg);
}
void start_timeOUT()
{
        timeoutSW_0.startIO(inputPin, inputPressed);
        timeoutSW_0.def_funcs(switchON_cb, switchOFF_cb);
}

void setup()
{
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
        startIO();
        start_timeOUT();

        if (OnatBoot)
        {
                timeoutSW_0.start_TO(timeoutSW_0.def_TO_minutes, "BootOn");
        }
}
void loop()
{
        iot.looper();
        timeoutSW_0.looper();
        delay(100);
}
