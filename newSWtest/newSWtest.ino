#include <myIOT2.h>
#include <smartSwitch.h>
#include "myIOT_settings.h"

smartSwitch SW;
char *MQTT_cmds[] = {"off", "on"};
char *SW_Types[]={"Button","MQTT","Timeout"};
// void smartSwitch::set_extON(char *msg, char *topic)
// {
//         iot.pub_noTopic(msg, topic, true);
// }

void send_virtCMD(smartSwitch &sw)
{
        if (sw.is_virtCMD())
        {
                char msg[100];
                iot.pub_noTopic(MQTT_cmds[sw.telemtryMSG.state], sw.name, true);
                sprintf(msg,"[%s]: Switched [%s] Virtual [%s]",SW_Types[sw.telemtryMSG.reason], MQTT_cmds[sw.telemtryMSG.state], sw.name);
                iot.pub_msg(msg);
        }
}

void init_smartSW()
{
        SW.set_input(D1, 2);
        SW.set_output(D4);
        SW.set_name("myHome/Light/SW0");
        SW.set_timeout(5);
        SW.get_prefences();
}

void setup()
{
        startIOTservices();
        init_smartSW();
}
void loop()
{
        iot.looper();
        if (SW.loop())
        {
                send_virtCMD(SW);
                SW.clear_newMSG();
                Serial.print("NEW_MSG: State:");
                Serial.print(SW.telemtryMSG.state);
                Serial.print("\tReason: ");
                Serial.println(SW.telemtryMSG.reason);
        }
        // Serial.println(SW.get_remain_time());
}
