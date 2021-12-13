#include <myTimeoutSwitch.h>

timeOUTSwitch TOswitch;

void startTO(uint8_t src, uint8_t i)
{
}
void endTO(uint8_t src, uint8_t i)
{
}

void TOsw_init()
{
    TOswitch.useInput = true;
    TOswitch.def_TO_minutes = 150;
    TOswitch.maxON_minutes = 240;
    TOswitch.trigType = 0;
    TOswitch.def_funcs(startTO, endTO);
    TOswitch.startIO(swPin);
}
