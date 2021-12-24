#include <Arduino.h>
#include <myTimeoutSwitch.h>

timeOUTSwitch TOswitch;
// extern void display_totalOnTime();
// extern int timeInc_counter;

void switchON_cb(uint8_t src, uint8_t i)
{
    char msg[100];
    char s1[25];
    char s2[7];
    char *srcs[] = {"Button", "Resume", "MQTT", "PowenOn"};

    iot.convert_epoch2clock(TOswitch.TO_duration, 0, s2, s1);
    if (digitalRead(RELAY1) != RelayOn || millis() < 15000) /* millis() added in case of power-on boot*/
    {
        digitalWrite(RELAY1, RelayOn);
        digitalWrite(indic_LEDpin, ledON);
        sprintf(msg, "%s: Switched [ON] for [%s]", srcs[src], s2);
    }
    else
    {
        sprintf(msg, "%s: add-Time [%s]", srcs[src], s2);
    }
    iot.pub_msg(msg);
}
void switchOFF_cb(uint8_t src, uint8_t i)
{
    char msg[100];
    char s1[15];
    char s2[7];
    char clk[25];
    char clk2[25];

    char *srcs[] = {"Button", "Timeout", "MQTT"};

    if (digitalRead(RELAY1) == RelayOn && TOswitch.onClk() != 0)
    {
        digitalWrite(RELAY1, !RelayOn);
        digitalWrite(indic_LEDpin, !ledON);

        int a = iot.now() - TOswitch.onClk();
        iot.convert_epoch2clock(a, 0, clk, s1);

        if (TOswitch.remTime() > 0) /* Forced-End before time */
        {
            iot.convert_epoch2clock(TOswitch.remTime(), 0, clk2, s1);
            sprintf(msg, "%s: Switched [OFF] after [%s], remained [%s]", srcs[src], clk, clk2);
        }
        else /* End by timeout */
        {
            sprintf(msg, "%s: Switched [OFF] ended after [%s]", srcs[src], clk);
        }
        iot.pub_msg(msg);
    }
}
void TOswitch_init()
{
    TOswitch.useInput = true;
    TOswitch.trigType = 4;
    TOswitch.maxON_minutes = maxTO;
    TOswitch.def_TO_minutes = timeIncrements;
    TOswitch.startIO(INPUT1, SwitchOn);
    TOswitch.def_funcs(switchON_cb, switchOFF_cb);
}
