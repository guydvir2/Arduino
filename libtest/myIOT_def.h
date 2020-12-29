// ~~~~~~~~~~~~~~ IOT ~~~~~~~~~~~~~
#if USE_IOT == 1
#include <myIOT2.h>
#define DEV_TOPIC "RF24_PORT"
#define PREFIX_TOPIC "myHome"
#define GROUP_TOPIC ""

myIOT2 iot;
extern int q_counter;
extern int a_counter;
extern bool send(const char *msg_t, const char *p0, const char *p1, const char *counter);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "Status: OK");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help: Commands #3 - [NEW]");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "Ver: Ver:%s", VER);
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (strcmp(iot.inline_param[0], "q") == 0)
        {
            q_counter++;
            char a[10];
            sprintf(a, "#%d", q_counter);
            send("q", iot.inline_param[1], a,"");
        }
        Serial.println(iot.inline_param[0]);
        Serial.println(iot.inline_param[1]);
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
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);
    iot.start_services(addiotnalMQTT);
}
#endif