#include "smartLights.h"

smartLights::smartLights()
{
}
void smartLights::set_inputs(uint8_t inputPin, uint8_t input_type)
{
    _SW.set_input(inputPin, input_type);
}
void smartLights::set_outputs(uint8_t outputPin, bool isON)
{
    _Lights.init(outputPin, isON);
}
void smartLights::set_outputs(uint8_t outputPin, int intPWMres, bool dim)
{
    _Lights.init(outputPin, intPWMres, dim);
}
void smartLights::set_timeout(int timeout)
{
    _SW.set_timeout(timeout);
}
bool smartLights::loop()
{
    return _SW.loop(); /* */
}

void smartLights::get_state()
{
}
bool smartLights::isPWM()
{
}
bool smartLights::isON()
{
}
