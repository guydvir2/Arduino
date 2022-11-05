#include <Arduino.h>
#include <myIOT2.h>
#include <smartSwitch.h>
#include "myIOT_settings.h"

smartSwitch smartSW;
const char *ver = "smartSwitch_v0.1";

void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin,
                  uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot)
{
        smartSW.set_id(id);
        smartSW.set_name(SWname);
        smartSW.set_output(output_pin); /* pin-255 calls a virtualCMD */
        smartSW.set_input(input_pin, butType);
        smartSW.set_indiction(indic_pin, indic_on);
        smartSW.set_timeout(timeout * 60);
        smartSW.set_useLockdown(uselckd);

        if (onatboot)
        {
                smartSW.turnON_cb(3);
        }
}
void update_MQTT_state(uint8_t i)
{
        char a[5];
        sprintf(a, "%s", i == 0 ? "off" : "on");
        iot.pub_state(a);
}
void smartSW_telemetry2MQTT()
{
        char msg[100];
        char msg2[25];
        char *sw_states[] = {"Off", "On"};
        char *origins[] = {"Switch", "Timeout", "MQTT", "BOOT"};

        if ((smartSW.useTimeout() && smartSW.telemtryMSG.state == 1) || (smartSW.useTimeout() && smartSW.telemtryMSG.state == 0 && smartSW.get_remain_time() != 0))
        {
                char a[15];
                iot.convert_epoch2clock(smartSW.get_remain_time() / 1000, 0, a);
                sprintf(msg2, ", timeout [%s]", a);
        }
        else
        {
                strcpy(msg2, "");
        }
        sprintf(msg, "[%s]: turned [%s]%s", origins[smartSW.telemtryMSG.reason], sw_states[smartSW.telemtryMSG.state], msg2);
        iot.pub_msg(msg);

        update_MQTT_state(smartSW.telemtryMSG.state);
}
void print_props()
{
        SW_props props;
        smartSW.get_SW_props(props);

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
}

void setup()
{
        startIOTservices();
}
void loop()
{
        iot.looper();
        if (smartSW.loop())
        {
                smartSW_telemetry2MQTT();
                smartSW.clear_newMSG();
        }
}
