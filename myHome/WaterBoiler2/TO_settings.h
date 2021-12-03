#include <myTimeoutSwitch.h>

timeOUTSwitch TOswitch;
extern void display_totalOnTime();
extern unsigned long onclk;

void turnON_cb(uint8_t src, uint8_t i)
{
    char m[100];
    char c[40];
    char *srcs[] = {"Button", "Timeout", "MQTT"};
    sec2clock(TOswitch.TO_duration, c);

    sprintf(m, "[%s]: Timeout started [%s]", srcs[src], c);
    iot.pub_msg(m);
    digitalWrite(RELAY1, RelayOn);
}
void turnOFF_cb(uint8_t src, uint8_t i)
{
    if (TOswitch.inTO)
    {
        char m[100];
        char c[30];
        char *srcs[] = {"Button", "Timeout", "MQTT"};
        display_totalOnTime();
        if (TOswitch.remTime() > 0)
        {
            sec2clock(TOswitch.TO_duration - TOswitch.remTime(), c);
            sprintf(m, "[%s]: Timeout ended after [%s]", srcs[src], c);
        }
        else
        {
            sec2clock(TOswitch.TO_duration, c);
            sprintf(m, "[%s]: Timeout finished [%s]", srcs[src], c);
        }
        iot.pub_msg(m);
        digitalWrite(RELAY1, !RelayOn);
        onclk = 0;
    }
}

void TOswitch_init()
{
    TOswitch.useInput = true;
    TOswitch.trigType = 0;
    TOswitch.def_funcs(turnON_cb, turnOFF_cb);
}
