#include <myIOT2.h>

#define DEV_TOPIC "mailServer"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"
#define MAIL_MQTT_TOPIC PREFIX_TOPIC "/email"

myIOT2 iot;
extern void construct_2email(const char *from, const char *subj, const char *msg);
extern const char *mserver_ver;

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "Status: ");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", mserver_ver, iot.ver, isESP8266? "ESP8266":"ESP32");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [email,[<msg>,<subject>,<from>]]");
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);

        if (strcmp(iot.inline_param[0], "email") == 0)
        {
            construct_2email(iot.inline_param[3], iot.inline_param[2], iot.inline_param[1]);
        }
        for (int i = 0; i < iot.num_param; i++)
        {
            strcpy(iot.inline_param[i], "");
        }
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = true;
    iot.resetFailNTP = true;
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);
    strcpy(iot.extTopic, MAIL_MQTT_TOPIC);

    iot.start_services(addiotnalMQTT);
}
