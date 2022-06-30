#include <myIOT2.h>

#define DEV_TOPIC "RF_HUB"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

myIOT2 iot;

extern void send_RF_msg(char *msg);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "BOOOOO");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2:No other functions");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2:");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "RFmsg") == 0)
    {
        send_RF_msg("FUK_ME");
    }
    else
    {
        if (iot.num_p > 1)
        {
            if (strcmp(iot.inline_param[0], "sendRF") == 0)
            {
                char qwerty[MAX_PAYLOAD_SIZE];
                sprintf(qwerty, "%s;%s", iot.inline_param[1], iot.inline_param[2]);
                send_RF_msg(qwerty);
            }
        }
    }
}
void startIOTservices()
{
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useDebug = true;
    iot.useSerial = true;
    iot.useextTopic = false;
    iot.useResetKeeper = true;
    iot.useNetworkReset = true;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;

    iot.debug_level = 0;
    iot.noNetwork_reset = 10;

    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

    //     iot.extTopic_msgArray[0] = &extTopic_msg;
    //     iot.extTopic[0] = "myHome/new";
    // #else
    //     iot.useFlashP = true;
    // #endif
    iot.start_services(addiotnalMQTT);
}

