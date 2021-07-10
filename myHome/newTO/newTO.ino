#include <myIOT2.h>
#include <Arduino.h>
#include <myTimeoutSwitch.h>

myIOT2 iot;

timeOUTSwitch *TOsw[2] = {};

/* Values get uodated from parameter files */
// char *name_0 = "SW_A";
// char *name_1 = "SW_B";
int PWM_res = 1023;
bool inputPressed[] = {LOW, LOW};
bool output_ON[] = {HIGH, HIGH};
bool OnatBoot[] = {true, true};
byte numSW = 2;
byte inputPin[] = {5, 2};
byte outputPin[] = {4, 0};
byte defPWM[] = {2, 2};
byte limitPWM[] = {80, 80};
char sw_names[2][10]; // = {name_0, name_1};

// bool useInput[] = {true, true};
// byte totPWMsteps[] = {3, 3};
// byte trigType[] = {0, 0};
// int def_TO_minutes[] = {1, 1};
// int maxON_minutes[] = {720, 600};

/* End */

#include "myTO_param.h"
#include "myIOT_settings.h"

void startIO()
{
        for (int x = 0; x < numSW; x++)
        {
                pinMode(outputPin[x], OUTPUT);
                digitalWrite(outputPin[x], !output_ON);
        }
}
bool get_SWstate(byte i = 0)
{
        if (TOsw[i]->trigType != 3)
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
int convDim(byte dim_step, byte i)
{
        return (int)((dim_step * PWM_res * limitPWM[i]) / (TOsw[i]->totPWMsteps * 100));
}
void PWMdim(int dim_step, byte i)
{
        byte delay_step = 2;
        int C_val = 1;
        int desiredval = 0;
        static int _last_dimVal[] = {0, 0};
        desiredval = convDim(dim_step, i);

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
void switchON_cb(char msg1[50], byte i)
{
        char msg[100];
        char s1[25];
        char s2[7];
        char clk[25];

        if (TOsw[i]->trigType != 3)
        {
                if (get_SWstate(i) == 0)
                {
                        iot.convert_epoch2clock(TOsw[i]->TO_duration_minutes * 60, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        switchIt(HIGH, i);
                        sprintf(msg, "%s: [%s] Switched [ON] for [%s]", msg1, sw_names[i], clk);
                        iot.pub_msg(msg);
                }
        }
        else
        {
                if (TOsw[i]->pwm_pCount == 0)
                {
                        TOsw[i]->pwm_pCount = 1;
                }
                PWMdim(TOsw[i]->pwm_pCount, i);

                iot.convert_epoch2clock(TOsw[i]->TO_duration_minutes * 60, 0, s1, s2);
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

        if (get_SWstate(i) == 1)
        {
                if (TOsw[i]->trigType != 3)
                {
                        Serial.println("A");
                        switchIt(LOW, i);
                }
                else
                {
                        Serial.println("B");
                        PWMdim(0, i);
                        Serial.println("C");
                }
                // int a = (int)((millis() - TOsw[i]->TO_start_millis) / 1000);
                // if (TOsw[i]->remTime() > 0)
                // {
                //         iot.convert_epoch2clock(a, 0, s1, s2);
                //         simplifyClock(s2, s1, clk);
                //         iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                //         simplifyClock(s2, s1, clk2);
                //         sprintf(msg, "%s: [%s] Switched [OFF] after [%s], remained [%s]", msg1, sw_names[i], clk, clk2);
                // }
                // else
                // {
                //         iot.convert_epoch2clock(a, 0, s1, s2);
                //         simplifyClock(s2, s1, clk);
                //         sprintf(msg, "%s: [%s] Switched [OFF] ended after [%s]", msg1, sw_names[i], clk);
                // }
        }
        // iot.pub_msg(msg);
}
void init_timeOUT()
{
        static timeOUTSwitch timeoutSW_0;
        TOsw[0] = &timeoutSW_0;
        TOsw[0]->icount = 0;
        if (numSW == 2)
        {
                static timeOUTSwitch timeoutSW_1;
                TOsw[1] = &timeoutSW_1;
                TOsw[1]->icount = 1;
        }
}
void start_timeOUT()
{
        for (int i = 0; i < numSW; i++)
        {
                // TOsw[i]->useInput = useInput[i];
                // TOsw[i]->maxON_minutes = maxON_minutes[i];
                // TOsw[i]->def_TO_minutes = def_TO_minutes[i];
                // TOsw[i]->trigType = trigType[i];
                // TOsw[i]->totPWMsteps = totPWMsteps[i];
                // strcpy(sw_names[i], DOC["sw_names"][i].as<const char *>());

                TOsw[i]->startIO(inputPin[i], inputPressed[i]);
                TOsw[i]->def_funcs(switchON_cb, switchOFF_cb);
                if (OnatBoot[i])
                {
                        TOsw[i]->start_TO(TOsw[i]->def_TO_minutes, "Boot-On");
                }
                Serial.print("Start Switch #");
                Serial.println(i);
        }
}

void setup()
{
        init_timeOUT();
        startRead_parameters();
        startIOTservices();
        startIO();
        start_timeOUT();
        endRead_parameters();
}
void loop()
{
        iot.looper();
        for (int i = 0; i < numSW; i++)
        {
                TOsw[i]->looper();
        }
        delay(100);
}
