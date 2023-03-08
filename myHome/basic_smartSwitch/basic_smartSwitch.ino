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
const char *ver = "smartSwitch_v0.4";

#include "myIOT_settings.h"
#include "parameterRead.h"

void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t pwm_pwr,
                  uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot)
{
        if (id == 1)
        {
                totSW++;
        }
        smartSwArray[id]->set_name(SWname);
        smartSwArray[id]->set_output(output_pin, pwm_pwr); /* pin-255 calls a virtualCMD, pwm_pwr>0 create PWM output */
        smartSwArray[id]->set_input(input_pin, butType);
        smartSwArray[id]->set_indiction(indic_pin, indic_on);
        smartSwArray[id]->set_timeout(timeout * 60);
        smartSwArray[id]->set_useLockdown(uselckd);
        smartSwArray[id]->set_id(id); /* have to be here to avoid wrong ID number */

        if (onatboot)
        {
                smartSwArray[id]->turnON_cb(3);
        }
        else
        {
                smartSwArray[id]->turnOFF_cb(3);
        }
        print_props(id);
}
void update_MQTT_state(uint8_t i, uint8_t x = 0)
{
        char a[5];
        sprintf(a, "%s", i == 0 ? "off" : "on");
        iot.pub_state(a, x);
}
void smartSW_telemetry2MQTT(uint8_t i)
{
        char msg[150];
        char msg2[30]{};
        const char *sw_states[] = {"Off", "On"};
        const char *origins[] = {"Switch", "Timeout", "MQTT", "BOOT"};

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

        SW_props props;
        smartSwArray[i]->get_SW_props(props);
        if (totSW > 1)
        {
                sprintf(msg, "[%s]: [%s] turned [%s]%s", origins[smartSwArray[i]->telemtryMSG.reason], props.name, sw_states[smartSwArray[i]->telemtryMSG.state], msg2);
        }
        else
        {
                sprintf(msg, "[%s]: turned [%s]%s", origins[smartSwArray[i]->telemtryMSG.reason], sw_states[smartSwArray[i]->telemtryMSG.state], msg2);
        }

        iot.pub_msg(msg);
        update_MQTT_state(smartSwArray[i]->telemtryMSG.state, i);
}
void print_props(uint8_t i)
{
        SW_props props;
        smartSwArray[i]->get_SW_props(props);

        Serial.print("<<<<<<< smartSwitch #");
        Serial.print(props.id);
        Serial.println(" >>>>>>>");
        Serial.print("> Type:\t\t");
        Serial.print(props.type);
        Serial.println("  [0:None, 1:On-Off, 2:pushButton]");
        Serial.print("> Name:\t\t");
        Serial.println(props.name);
        Serial.print("> inputPin:\t");
        Serial.println(props.inpin);
        Serial.print("> outputPin:\t");
        Serial.println(props.outpin);
        Serial.print("> usetimeOut:\t");
        Serial.println(props.timeout);
        Serial.print("> virtualCMD:\t");
        Serial.println(props.virtCMD);
        Serial.print("> useLockdown:\t");
        Serial.println(props.lockdown);

        Serial.print("<<<<<<< END ");
        Serial.println(">>>>>>");
        Serial.flush();
}

void setup()
{
        Serial.begin(115200);
        read_all_parameters();
        startIOTservices();
}
void loop()
{
        iot.looper();
        for (uint8_t i = 0; i < totSW; i++)
        {
                if (smartSwArray[i]->loop())
                {
                        smartSW_telemetry2MQTT(i);
                        smartSwArray[i]->clear_newMSG();
                }
        }
}
