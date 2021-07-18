#include <myIOT2.h>

myIOT2 iot;

#define DEV_TOPIC "empty"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "BOOOOO");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
        // iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [; m; ,x]");
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
    iot.resetFailNTP = true;
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

    iot.start_services(addiotnalMQTT);
}
