myIOT2 iot;
#include <myTimeoutSwitch.h>

extern timeOUTSwitch TOswitch;
extern void sec2clock(int sec, char *output_text);
extern unsigned long onclk;

void giveStatus(char *outputmsg)
{
    char t1[50];
    char t2[50];
    char t3[50];

    if (TOswitch.remTime() > 0)
    {
        sec2clock(TOswitch.remTime(), t2);
        sec2clock(TOswitch.TO_duration, t3);
        sprintf(t1, "timeLeft[%s], total[%s]", t2, t3);
    }
    else
    {
        sprintf(t1, "");
    }
    sprintf(t2, "[%s] %s", TOswitch.inTO ? "ON" : "OFF", t1);
    sprintf(outputmsg, "Status: %s", t2);
}
void addiotnalMQTT(char *income_msg)
{
    char msg_MQTT[150];
    char msg2[20];

    if (strcmp(income_msg, "status") == 0)
    {
        giveStatus(msg_MQTT);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "ver2") == 0)
    {
        sprintf(msg_MQTT, "ver #2: [%s], lib: [%s], timeoutSw[%s]", VEr, iot.ver, TOswitch.Ver);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "help2") == 0)
    {
        sprintf(msg_MQTT, "Help2: Commands #1 - [on, off, remain, {addTO,minutes}, {timeout,minutes}]");
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "off") == 0)
    {
        TOswitch.finish_TO(2);
    }
    else if (strcmp(income_msg, "on") == 0)
    {
        TOswitch.start_TO(maxTO, 2);
        onclk = TOswitch.onClk();
    }
    else if (strcmp(income_msg, "remain") == 0)
    {
        char s1[15], s2[20];
        char clk[60];
        if (TOswitch.remTime() > 0)
        {
            iot.convert_epoch2clock(TOswitch.remTime(), 0, s1, s2);
            // iot.get_timeStamp(TOswitch.onClk());
            sprintf(clk, "MQTT: remain [%s] ", s1);
            iot.pub_msg(clk);
        }
        else
        {
            sprintf(clk, "MQTT: remain [Off]");
            iot.pub_msg(clk);
        }
    }
    else
    {
        int num_parameters = iot.inline_read(income_msg);
        if (num_parameters > 1)
        {
            if (strcmp(iot.inline_param[0], "timeout") == 0)
            {
                TOswitch.start_TO(atoi(iot.inline_param[1]), 2);
                onclk = TOswitch.onClk();
            }
            else if (strcmp(iot.inline_param[0], "addTO") == 0)
            {
                TOswitch.add_TO(atoi(iot.inline_param[1]), 2);
            }
        }
    }
}

void startIOTservices()
{
    iot.useSerial = false;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = true;
    iot.useextTopic = false;
    iot.useDebug = true;
    iot.useFlashP = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 10;
    iot.useBootClockLog = true;
    iot.useAltermqttServer = false;
    iot.ignore_boot_msg = false;
    strcpy(iot.deviceTopic, DEVICE_TOPIC);
    strcpy(iot.prefixTopic, MQTT_PREFIX);
    strcpy(iot.addGroupTopic, MQTT_GROUP);

    iot.start_services(addiotnalMQTT);
}
