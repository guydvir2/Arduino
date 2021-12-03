myIOT2 iot;
#include <myTimeoutSwitch.h>

extern timeOUTSwitch TOswitch;
extern void turnON_cb(uint8_t src, uint8_t i);
extern void turnOFF_cb(uint8_t src, uint8_t i);
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
        sprintf(t1, "timeLeft[%s], total[%s]", t2,t3);
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
        sprintf(msg_MQTT, "ver #2: myBoiler[%s]", VEr);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "help2") == 0)
    {
        sprintf(msg_MQTT, "Help2: Commands #1 - [on, off, flash, format]");
        iot.pub_msg(msg_MQTT);
        sprintf(msg_MQTT, "Help2: Commands #2 - [remain; restartTO; timeout,x; updateTO,x; restoreTO, statusTO]");
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
        }
    }
}

void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = true;
    iot.useextTopic = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 30;
    iot.useBootClockLog = true;
    iot.useAltermqttServer = false;
    iot.ignore_boot_msg = false;
    iot.deviceTopic = DEVICE_TOPIC;
    iot.prefixTopic = MQTT_PREFIX;
    iot.addGroupTopic = MQTT_GROUP;

    iot.start_services(addiotnalMQTT);
}
