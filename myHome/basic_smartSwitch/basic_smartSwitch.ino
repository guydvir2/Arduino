#include <Arduino.h>
#include <myIOT2.h>
#include <smartSwitch.h>

#define MAX_SW 2
#define MAN_MODE false

smartSwitch smartSW;
smartSwitch smartSW2;
smartSwitch *smartSwArray[MAX_SW] = {&smartSW, &smartSW2};

uint8_t totSW = 1;
bool bootProcess_OK = false;
const char *ver = "smartSwitch_v0.5";

#include "myIOT_settings.h"
#include "parameterRead.h"

void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t pwm_pwr,
                  uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot)
{
        if (id == 1)
        {
                totSW++;
        }
        smartSwArray[id]->useDebug = false;
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

void smartSW_telemetry2MQTT(uint8_t i)
{
        char msg[150];
        char msg2[30]{};
        char msg3[30]{};
        const char *sw_states[] = {"Off", "On"};
        const char *origins[] = {"Switch", "Timeout", "MQTT", "BOOT"};

        SW_props props;
        smartSwArray[i]->get_SW_props(props);

        /* Using timeout ?*/
        if ((smartSwArray[i]->useTimeout() && smartSwArray[i]->telemtryMSG.state == 1) || (smartSwArray[i]->useTimeout() && smartSwArray[i]->telemtryMSG.state == 0 && smartSwArray[i]->get_remain_time() != 0))
        {
                char a[10];
                iot.convert_epoch2clock(smartSwArray[i]->get_remain_time() / 1000, 0, a);
                sprintf(msg2, ", timeout [%s]", a);
        }
        else
        {
                strcpy(msg2, "");
        }

        /* PWM output ?*/
        if (props.PWM && smartSwArray[i]->telemtryMSG.state == 1)
        {
                sprintf(msg3, ", Power[%d%%]", smartSwArray[i]->telemtryMSG.pwm);
        }
        else{
                strcpy(msg3, "");
        }
        
        sprintf(msg, "[%s]: [%s] turned [%s]%s%S", origins[smartSwArray[i]->telemtryMSG.reason],
                props.name, sw_states[smartSwArray[i]->telemtryMSG.state], msg3,msg2);

        iot.pub_msg(msg);
        update_MQTT_state(smartSwArray[i]->telemtryMSG.state, i);
}
void smartSW_loop()
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

void setup()
{
        read_all_parameters();
        startIOTservices();
}
void loop()
{
        iot.looper();
        smartSW_loop();
}
