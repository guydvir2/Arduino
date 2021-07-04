#include <myIOT2.h>
#include <myTimeoutSwitch.h>

myIOT2 iot;

#define DEV_TOPIC "test"
#define GROUP_TOPIC "intLights"
#define PREFIX_TOPIC "myHome"

extern timeOUTSwitch timeoutSW_0;
extern void simplifyClock(char *days, char *clk, char retVal[25]);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        bool Q = digitalRead(outputPin);
        // int a = (int)((millis() - timeoutSW_0.TO_start_millis) / 1000);
        char s1[15];
        char s2[7];
        char clk[25];
        if (timeoutSW_0.remTime() > 0)
        {
            iot.convert_epoch2clock(timeoutSW_0.remTime(), 0, s1, s2);
            simplifyClock(s2, s1, clk);
        }
        else
        {
            sprintf(clk, "None");
        }
        if (timeoutSW_0.trigType != 3)
        {
            sprintf(msg, "Status: [%s] remain[%s]", Q == output_ON ? "ON" : "OFF", clk);
        }
        else
        {
            sprintf(msg, "Status: [%s] Power[%d%%] remain[%s]", timeoutSW_0.pwm_pCount > 0 ? "ON" : "OFF",
                    (int)(timeoutSW_0.pwm_pCount * 100 / timeoutSW_0.totPWMsteps), clk);
        }
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "show_flash_param") == 0)
    {
        char temp[300];
        char temp3[350];
        char *a[] = {iot.myIOT_paramfile, sketch_paramfile};
        iot.pub_debug("~~~Start~~~");
        for (int e = 0; e < sizeof(a) / sizeof(a[0]); e++)
        {
            strcpy(temp, iot.export_fPars(a[e], paramJSON));
            sprintf(temp3, "%s: %s", a[e], temp);
            iot.pub_debug(temp3);
            paramJSON.clear();
        }
        iot.pub_debug("~~~End~~~");
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
        // iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [on, off,{timeout,_sec}],show_flash_param");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "on") == 0)
    {
        timeoutSW_0.start_TO(timeoutSW_0.maxON_minutes, "MQTT");
    }
    else if (strcmp(incoming_msg, "off") == 0)
    {
        timeoutSW_0.finish_TO("MQTT");
    }
    else if (strcmp(incoming_msg, "remain") == 0)
    {
        int t = timeoutSW_0.remTime();
        sprintf(msg, "Remain: [%d]", t);
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (num_parameters > 1)
        {
            if (strcmp(iot.inline_param[0], "timeout") == 0)
            {
                if (timeoutSW_0.trigType != 3)
                {
                    timeoutSW_0.start_TO(atoi(iot.inline_param[1]), "MQTT");
                }
                else
                {
                    timeoutSW_0.pwm_pCount = atoi(iot.inline_param[2]);
                    timeoutSW_0.start_TO(atoi(iot.inline_param[1]), "MQTT");
                }
            }
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
    iot.useBootClockLog = paramJSON["useBootClockLog"];
    strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
    strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
    strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
    iot.start_services(addiotnalMQTT);
}
