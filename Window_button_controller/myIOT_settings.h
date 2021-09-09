#include <myIOT2.h>

myIOT2 iot;

#define DEV_TOPIC "AllWins"
#define GROUP_TOPIC "Windows"
#define PREFIX_TOPIC "myHome"

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "I'm OK");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: lib: [%s]",iot.ver);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: none");
        iot.pub_msg(msg);
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = false;
    iot.useDebug = true;
    iot.debug_level = 1;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 10;
    iot.useBootClockLog = true;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

    iot.start_services(addiotnalMQTT);
}
