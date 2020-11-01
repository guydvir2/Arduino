#include <myIOT.h>
#define DEBUG_TOPIC "debug"

myIOT iot;
extern esp8266Sleep espSleep;
extern void wait4OTA();
extern char *boardType;
extern const char *espVer;

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        // sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
        iot.pub_msg("NOT Sleeping");
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", espVer, VER, boardType);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help: Commands #2 - [status; m; delay,x]");
        iot.pub_msg(msg);
    }
    else if (strcmp(iot.mqqt_ext_buffer[1], "") != 0) /* message to debug topic*/
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (strcmp(iot.mqqt_ext_buffer[1], "m") == 0) /* maintainance*/
        {
            wait4OTA();
            iot.pub_ext("OTA", "", true);
        }
        else if (strcmp(iot.inline_param[0], "delay") == 0) /*postpone sleep */
        {
            espSleep.delay_sleep(atoi(iot.inline_param[1]));
            sprintf(msg, "Sleep: Postpone [%s sec]", iot.inline_param[1]);
            iot.pub_log(msg);
            iot.pub_ext("DELAY", "", true);
            for (int n = 0; n <= num_parameters - 1; n++)
            {
                sprintf(iot.inline_param[n], "");
            }
        }
        for (int i = 0; i < 3; i++)
        {
            sprintf(iot.mqqt_ext_buffer[i], "");
        }
    }
}
void startIOTservices()
{
    iot.useSerial = paramJSON["useSerial"];
    iot.useWDT = paramJSON["useWDT"];
    iot.useOTA = paramJSON["useOTA"];
    iot.useResetKeeper = paramJSON["useResetKeeper"];
    iot.resetFailNTP = paramJSON["useFailNTP"];
    iot.useDebug = paramJSON["useDebugLog"];
    iot.debug_level = paramJSON["debug_level"];
    iot.useNetworkReset = paramJSON["useNetworkReset"];
    iot.noNetwork_reset = paramJSON["noNetwork_reset"];
    iot.useextTopic = paramJSON["useextTopic"];
    strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
    strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
    strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);

    char a[50];
    sprintf(a, "%s/%s/%s/%s", iot.prefixTopic, iot.addGroupTopic, iot.deviceTopic, DEBUG_TOPIC);
    strcpy(iot.extTopic, a);
    iot.start_services(addiotnalMQTT);
}
