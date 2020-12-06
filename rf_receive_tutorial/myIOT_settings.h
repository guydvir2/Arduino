#include <myIOT2.h>

myIOT2 iot;


#define DEV_TOPIC "NRF24"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"


void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        // sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
        iot.pub_msg("NOT Sleeping");
    }
    // else if (strcmp(incoming_msg, "ver2") == 0)
    // {
    //     sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", espVer, VER, boardType);
    //     iot.pub_msg(msg);
    // }
    // else if (strcmp(incoming_msg, "help2") == 0)
    // {
    //     sprintf(msg, "Help: Commands #2 - [status; m; delay,x]");
    //     iot.pub_msg(msg);
    // }
    // else if (strcmp(iot.mqqt_ext_buffer[1], "") != 0) /* message to debug topic*/
    // {
    //     int num_parameters = iot.inline_read(incoming_msg);
    //     if (strcmp(iot.mqqt_ext_buffer[1], "m") == 0) /* maintainance*/
    //     {
    //         wait4OTA();
    //         iot.pub_ext("OTA", "", true);
    //     }
    //     else if (strcmp(iot.inline_param[0], "delay") == 0) /*postpone sleep */
    //     {
    //         espSleep.delay_sleep(atoi(iot.inline_param[1]));
    //         sprintf(msg, "Sleep: Postpone [%s sec]", iot.inline_param[1]);
    //         iot.pub_log(msg);
    //         iot.pub_ext("DELAY", "", true);
    //         for (int n = 0; n <= num_parameters - 1; n++)
    //         {
    //             sprintf(iot.inline_param[n], "");
    //         }
    //     }
    //     for (int i = 0; i < 3; i++)
    //     {
    //         sprintf(iot.mqqt_ext_buffer[i], "");
    //     }
    // }
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
