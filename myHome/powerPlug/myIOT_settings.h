#include <myIOT2.h>

#define                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh "multiSensor"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

myIOT2 iot;

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        iot.pub_msg("NOT Sleeping");
    }
}
void startIOTservices()
{
    iot.useSerial = paramJSON["useSerial"];
    iot.useWDT = paramJSON["useWDT"];
    iot.useOTA = paramJSON["useOTA"];
    iot.useResetKeeper = paramJSON["useResetKeeper"];
    iot.useDebug = paramJSON["useDebugLog"];
    iot.debug_level = paramJSON["debug_level"];
    iot.useNetworkReset = paramJSON["useNetworkReset"];
    iot.noNetwork_reset = paramJSON["noNetwork_reset"];
    iot.useextTopic = paramJSON["useextTopic"];
    iot.useBootClockLog = paramJSON["useBootClockLog"];
    strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
    strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
    strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);

    iot.start_services(addiotnalMQTT);
}
