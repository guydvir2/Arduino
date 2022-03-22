#include <Arduino.h>
#include <myTimeoutSwitch.h>

timeOUTSwitch timeoutSW_0;
timeOUTSwitch *TOsw[2] = {&timeoutSW_0, nullptr}; /* Support up to 2 TOsw */


/* ~~~~~~~~~~~~~~~~~~~~~~ Values get updated from parameter file ~~~~~~~~~~~~~~~~~~ */
int PWM_res = 1023;
int sketch_JSON_Psize = 1250;     /* Pass JSON size for Flash Parameter*/
bool inputPressed[] = {LOW, LOW}; /* High or LOW on button press */
bool output_ON[] = {HIGH, HIGH};  /* OUTPUT when ON is HIGH or LOW */
bool OnatBoot[] = {false, false}; /* After reboot- On or Off */
bool outputPWM[] = {false, false};
bool useIndicLED[] = {false, false}; /* use indication leds when ON*/
bool indic_ON[] = {true, true};

uint8_t numSW = 1;            /* Num of switches: 1 or 2 */
uint8_t inputPin[] = {3, 0};  /* IO for inputs */
uint8_t outputPin[] = {5, 2}; /* IO for outputs */
uint8_t indicPin[] = {4, 2};  /* IO for idication LEDS */
uint8_t defPWM[] = {2, 2};     /* Default PWM value for some cases not specified */
uint8_t limitPWM[] = {80, 80}; /* Limit total intensity, 1-100 */
char sw_names[2][20];          /* Name of each Switch, as shown on MQTT msg */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const char *VER = "TOswitch_v1.45";
#include "myTO_param.h"
#include "myIOT_settings.h"

// /* Controlling IOs */
void startOUTPUTio()
{
        analogWriteRange(PWM_res); /* PWM at ESP8266 */
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

                if (useIndicLED[x])
                {
                        pinMode(indicPin[x], OUTPUT);
                }
        }
}
bool get_SWstate(uint8_t i = 0)
{
        if (!outputPWM[i])
        {
                return digitalRead(outputPin[i]) == output_ON[i];
        }
        else /* PWM */
        {
                return TOsw[i]->pCounter > 0;
        }
}
bool switchIt(bool state, uint8_t i)
{
        if (state != get_SWstate(i))
        {
                if (state == HIGH) // High reffered as ON
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
int convDim(uint8_t dim_step, uint8_t i)
{
        return (int)((dim_step * PWM_res * limitPWM[i]) / (TOsw[i]->max_pCount * 100));
}
void PWMdim(int dim_step, uint8_t i)
{
        int C_val = 1;
        const uint8_t delay_step = 1;
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

// /* Config Switches instances */
void simplifyClock(char *days, char *clk, char *retVal)
{
        sprintf(retVal, "%s%s", strcmp(days, "0d") != 0 ? strcat(days, " ") : "", clk);
}
void switchON_cb(uint8_t src, uint8_t i)
{
        char msg[150];
        char s1[25];
        char s2[7];
        char clk[25];

        char *srcs[] = {"Button", "Resume", "MQTT", "PowenOn"};

        if (!outputPWM[i])
        {
                iot.convert_epoch2clock(TOsw[i]->TO_duration, 0, s1, s2);
                simplifyClock(s2, s1, clk);
                if (get_SWstate(i) == 0 || millis() < 15000) /* millis() added in case of power-on boot*/
                {
                        switchIt(HIGH, i);
                        sprintf(msg, "%s: [%s] Switched [ON] for [%s]", srcs[src], sw_names[i], clk);
                }
                else if (TOsw[i]->trigType == 2) /* Case of Sensor - extend duration */
                {
                        if (TOsw[i]->inTO == true)
                        {
                                return; /* To avoid posting mqtt msg. Timeout was updated in library */
                        }
                }
                else
                {
                        sprintf(msg, "%s: [%s] add-Time [%s]", srcs[src], sw_names[i], clk);
                }
                iot.pub_msg(msg);
        }
        else
        {
                bool msg_a = false;
                // ~~~~~~~~~ Setting intensity ~~~~~~~
                if (src == 0)
                {
                        if (TOsw[i]->pCounter == 0)
                        {
                                TOsw[i]->pCounter = defPWM[i];
                        }
                }
                else if (src == 1) /* Resume after boot - and time remain */
                {
                        TOsw[i]->pCounter = TOsw[i]->getCount(true);

                        if (TOsw[i]->pCounter == 0)
                        {
                                TOsw[i]->pCounter = defPWM[i];
                        }
                }

                else if ((TOsw[i]->trigType <= 1)) /* When setting Switch or button to use DEF value only */
                {
                        TOsw[i]->pCounter = defPWM[i];
                }
                else if (TOsw[i]->trigType == 2) /* Case of Sensor - extend duration */
                {
                        TOsw[i]->pCounter = defPWM[i];
                        if (TOsw[i]->inTO == true)
                        {
                                return;
                        }
                }
                else if (TOsw[i]->trigType == 3) /* Case of Button */
                {
                        if (TOsw[i]->inTO == true)
                        {
                                sprintf(msg, "%s: [%s] Power change[%d%%]", srcs[src], sw_names[i], (int)(100 * TOsw[i]->pCounter / TOsw[i]->max_pCount));
                                msg_a = true;
                        }
                }

                PWMdim(TOsw[i]->pCounter, i);

                if (msg_a == false) /* Valid for power on */
                {
                        iot.convert_epoch2clock(TOsw[i]->TO_duration, 0, s1, s2);
                        simplifyClock(s2, s1, clk);
                        sprintf(msg, "%s: [%s] Switched [ON] Power[%d%%] for [%s]", srcs[src], sw_names[i], (int)(100 * TOsw[i]->pCounter / TOsw[i]->max_pCount), clk);
                }
                iot.pub_msg(msg);
        }
        iot.pub_state("[ON]", i);
}
void switchOFF_cb(uint8_t src, uint8_t i)
{
        char msg[150];
        char s1[15];
        char s2[7];
        char clk[25];
        char clk2[25];

        char *srcs[] = {"Button", "Timeout", "MQTT"};

        if (get_SWstate(i) == 1 && TOsw[i]->onClk() != 0)
        {
                if (!outputPWM[i])
                {
                        switchIt(LOW, i);
                }
                else
                {
                        PWMdim(0, i);
                }

                int a = iot.now() - TOsw[i]->onClk();
                iot.convert_epoch2clock(a, 0, s1, s2);
                simplifyClock(s2, s1, clk);

                if (TOsw[i]->remTime() > 0) /* Forced-End before time */
                {
                        iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                        simplifyClock(s2, s1, clk2);
                        sprintf(msg, "%s: [%s] Switched [OFF] after [%s], remained [%s]", srcs[src], sw_names[i], clk, clk2);
                }
                else /* End by timeout */
                {
                        sprintf(msg, "%s: [%s] Switched [OFF] ended after [%s]", srcs[src], sw_names[i], clk);
                }
                iot.pub_msg(msg);
                iot.pub_state("[OFF]", i);
        }
}
void init_timeOUT()
{
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
                if (OnatBoot[i] && TOsw[i]->remTime() == 0) /* Turn on*/
                {
                        TOsw[i]->start_TO(TOsw[i]->def_TO_minutes, 3);
                }
        }
}
void loop_timeOUT()
{
        for (int i = 0; i < numSW; i++)
        {
                TOsw[i]->looper();
                if (useIndicLED[i])
                {
                        if (digitalRead(outputPin[i]) == output_ON[i])
                        {
                                digitalWrite(indicPin[i], indic_ON[i]);
                        }
                        else
                        {
                                digitalWrite(indicPin[i], !indic_ON[i]);
                        }
                }
        }
}

void setup()
{
        read_flashParameter();
        startOUTPUTio();
        startIOTservices();
        start_timeOUT();
}
void loop()
{
        iot.looper();
        loop_timeOUT(); // contains 50 ms delay in loop to avoid dbounce.
}

