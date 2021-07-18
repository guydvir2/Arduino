#include <myIOT2.h>
#include <Arduino.h>
#include <myTimeoutSwitch.h>

myIOT2 iot;

timeOUTSwitch *TOsw[2] = {}; /* Support up to 2 TOsw */

/* Values get uodated from parameter file */
int PWM_res = 1023;
bool inputPressed[] = {LOW, LOW};
bool output_ON[] = {HIGH, HIGH};
bool OnatBoot[] = {true, true};
byte numSW = 2;
byte inputPin[] = {5, 2};
byte outputPin[] = {4, 0};
byte defPWM[] = {2, 2};
byte limitPWM[] = {80, 80};
char sw_names[2][10];
/* End */

const char *VER = "TOswitch_v0.3";

#include "myTO_param.h"
#include "myIOT_settings.h"

/* Controlling IOs */
void startIO()
{
        for (int x = 0; x < numSW; x++)
        {
                pinMode(outputPin[x], OUTPUT);
                if (OnatBoot[x])
                {
                        digitalWrite(outputPin[x], output_ON[x]);
                }
                else
                {
                        digitalWrite(outputPin[x], !output_ON[x]);
                }
        }
}
bool get_SWstate(byte i = 0)
{
        if (TOsw[i]->outputPWM == false)
        {
                if (digitalRead(outputPin[i]) == output_ON[i])
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
                if (TOsw[i]->pwm_pCount > 0)
                {
                        return 1;
                }
                else
                {
                        return 0;
                }
        }
}
bool switchIt(bool state, byte i)
{
        if (state != get_SWstate(i))
        {
                if (state == HIGH)
                {
                        digitalWrite(outputPin[i], output_ON[i]);
                }
                else
                {
                        digitalWrite(outputPin[i], !output_ON[i]);
                }
                return 1;
        }
        else
        {
                return 0;
        }
}
int convDim(byte dim_step, byte i)
{
        return (int)((dim_step * PWM_res * limitPWM[i]) / (TOsw[i]->totPWMsteps * 100));
}
void PWMdim(int dim_step, byte i)
{
        int C_val = 1;
        const byte delay_step = 2;
        static int _last_dimVal[] = {0, 0};
        int desiredval = convDim(dim_step, i);

        if (desiredval < _last_dimVal[i])
        {
                C_val = -1 * C_val;
        }
        while (abs(desiredval - _last_dimVal[i]) >= 1)
        {
                _last_dimVal[i] = _last_dimVal[i] + C_val;
                analogWrite(outputPin[i], _last_dimVal[i]);
                delay(delay_step);
        }
}

/* Config Switches instances */
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
void switchON_cb(char msg1[50], byte i)
{
        char msg[100];
        char s1[25];
        char s2[7];
        char clk[25];

        if (TOsw[i]->outputPWM == false)
        {
                if (get_SWstate(i) == 0 || millis() < 15000) /* millis() added in case of power-on boot*/
                {
                        iot.convert_epoch2clock(TOsw[i]->TO_duration, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        switchIt(HIGH, i);
                        sprintf(msg, "%s: [%s] Switched [ON] for [%s]", msg1, sw_names[i], clk);
                        iot.pub_msg(msg);
                }
        }
        else
        {
                if (TOsw[i]->trigType == 1)
                {
                        TOsw[i]->pwm_pCount = defPWM[i];
                }
                PWMdim(TOsw[i]->pwm_pCount, i);

                iot.convert_epoch2clock(TOsw[i]->TO_duration, 0, s1, s2);
                simplifyClock(s2, s1, clk);
                sprintf(msg, "%s: [%s] Switched [ON] Power[%d%%] for [%s]", msg1, sw_names[i], (int)(100 * TOsw[i]->pwm_pCount / TOsw[i]->totPWMsteps), clk);
                iot.pub_msg(msg);
        }
}
void switchOFF_cb(char msg1[50], byte i)
{
        char msg[100];
        char s1[15];
        char s2[7];
        char clk[25];
        char clk2[25];

        if (get_SWstate(i) == 1 && TOsw[i]->onClk() != 0)
        {
                if (TOsw[i]->outputPWM == false)
                {
                        switchIt(LOW, i);
                }
                else
                {
                        PWMdim(0, i);
                }

                int a = now() - TOsw[i]->onClk();
                iot.convert_epoch2clock(a, 0, s1, s2);
                simplifyClock(s2, s1, clk);

                if (TOsw[i]->remTime() > 0)
                {
                        iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                        simplifyClock(s2, s1, clk2);
                        sprintf(msg, "%s: [%s] Switched [OFF] after [%s], remained [%s]", msg1, sw_names[i], clk, clk2);
                        iot.pub_msg(msg);
                }
                else
                {
                        sprintf(msg, "%s: [%s] Switched [OFF] ended after [%s]", msg1, sw_names[i], clk);
                        iot.pub_msg(msg);
                }
        }
        else
        {
                sprintf(msg, "Error: [%s] not ON or not in timeout", sw_names[i]);
                iot.pub_msg(msg);
        }
}
void init_timeOUT()
{
        static timeOUTSwitch timeoutSW_0;
        TOsw[0] = &timeoutSW_0;
        if (numSW == 2)
        {
                static timeOUTSwitch timeoutSW_1;
                TOsw[1] = &timeoutSW_1;
        }
}
void start_timeOUT()
{
        for (int i = 0; i < numSW; i++)
        {
                TOsw[i]->icount = i;
                TOsw[i]->startIO(inputPin[i], inputPressed[i]);
                TOsw[i]->def_funcs(switchON_cb, switchOFF_cb);
                if (OnatBoot[i])
                {
                        if (TOsw[i]->remTime() == 0) /* last oper was ended prior to reboot */
                        {
                                TOsw[i]->start_TO(TOsw[i]->def_TO_minutes, "Boot-On");
                        }
                }
        }
}
void loop_timeOUT()
{
        for (int i = 0; i < numSW; i++)
        {
                TOsw[i]->looper();
        }
}

/* Mains */
void setup()
{
        init_timeOUT();
        startRead_parameters();
        startIO();
        startIOTservices();
        start_timeOUT();
        endRead_parameters();
}
void loop()
{
        iot.looper();
        loop_timeOUT();
        delay(50);
}
