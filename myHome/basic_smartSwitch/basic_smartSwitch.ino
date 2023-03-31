#include <Arduino.h>
#include <myIOT2.h>
#include <smartSwitch.h>

#define MAX_SW 2
#define MAN_MODE false
#define DEBUG_MODE true

smartSwitch smartSW;
smartSwitch smartSW2;
smartSwitch *smartSwArray[MAX_SW] = {&smartSW, &smartSW2};

uint8_t totSW = 1;
const char *ver = "smartSwitch_v0.65";

#include "pins.h"
#include "myIOT_settings.h"
#include "parameterRead.h"

void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t pwm_pwr,
                  uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot)
{
        if (id == 1)
        {
                totSW++;
        }
        smartSwArray[id]->useDebug = DEBUG_MODE;
        smartSwArray[id]->set_name(SWname);
        smartSwArray[id]->set_output(output_pin, pwm_pwr); /* pin-255 calls a virtualCMD, pwm_pwr>0 create PWM output */
        smartSwArray[id]->set_input(input_pin, butType);
        smartSwArray[id]->set_indiction(indic_pin, indic_on);
        smartSwArray[id]->set_timeout(timeout);
        smartSwArray[id]->set_useLockdown(uselckd);
        smartSwArray[id]->set_id(id); /* have to be here to avoid wrong ID number */

        if (onatboot)
        {
                smartSwArray[id]->turnON_cb(EXT_1);
        }
        else
        {
                smartSwArray[id]->turnOFF_cb(EXT_1);
        }
        smartSwArray[id]->print_preferences();
}
void smartSW_loop()
{
        if (bootProcess_OK)
        {
                for (uint8_t i = 0; i < totSW; i++)
                {
                        if (smartSwArray[i]->loop())
                        {
                                smartSW_telemetry2MQTT(i);
                                smartSwArray[i]->clear_newMSG();
                        }
                }
        }
}

void setup()
{
#if DEBUG_MODE
        Serial.begin(115200);
#endif
        getStored_parameters();
        startIOTservices();
}
void loop()
{
        iot.looper();
        smartSW_loop();
}
