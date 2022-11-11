#ifndef smartLights_h
#define smartLights_h
+
#include <Arduino.h>
#include <smartSwitch.h>
#include <TurnOnLights.h>

#ifndef UNDEF_PIN
#define UNDEF_PIN 255
#endif

class smartLights
{
public:
    smartLights();
    void set_inputs(uint8_t inputPin = UNDEF_PIN, uint8_t input_type = 0);
    void set_outputs(uint8_t outputPin = UNDEF_PIN, bool isON = true);
    void set_outputs(uint8_t outputPin = UNDEF_PIN, int intPWMres=1023, bool dim=false);
    void set_timeout(int timeout = 0);
    bool loop();

    void get_state();
    bool isPWM();
    bool isON();


private:
    smartSwitch _SW;
    TurnOnLights _Lights;
};
#endif
