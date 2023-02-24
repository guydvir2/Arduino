myIOT2 iot;
#include <myTimeoutSwitch.h>

extern timeOUTSwitch TOswitch;
// extern void sec2clock(int sec, char *output_text);
extern unsigned long onclk;

#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[3][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

void giveStatus(char *outputmsg)
{
    char t1[100];
    char t2[50];
    char t3[50];

    if (TOswitch.remTime() > 0)
    {
        // sec2clock(TOswitch.remTime(), t2);
        // sec2clock(TOswitch.TO_duration, t3);
        sprintf(t1, "timeLeft[%s], total[%s]", t2, t3);
    }
    else
    {
        sprintf(t1, " ");
    }
    sprintf(t2, "[%s] %s", TOswitch.inTO ? "ON" : "OFF", t1);
    sprintf(outputmsg, "Status: %s", t2);
}
void addiotnalMQTT(char *income_msg, char *topic)
{
    char msg_MQTT[150];

    if (strcmp(income_msg, "status") == 0)
    {
        giveStatus(msg_MQTT);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "ver2") == 0)
    {
        sprintf(msg_MQTT, "ver #2: [%s], timeoutSw[%s]", VEr, TOswitch.Ver);
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
        if (iot.num_p > 1)
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
    iot.useFlashP = false;
    iot.noNetwork_reset = 6;
    iot.ignore_boot_msg = false;

    iot.topics_gen_pub[0] = "myHome/Messages";
    iot.topics_gen_pub[1] = "myHome/log";
    iot.topics_gen_pub[1] = "myHome/debug";

    iot.topics_pub[0] = "myHome/WaterBoiler";
    iot.topics_pub[0] = "myHome/WaterBoiler/Avail";
    iot.topics_pub[1] = "myHome/WaterBoiler/State";
    iot.topics_sub[0] = "myHome/WaterBoiler";
    iot.topics_sub[1] = "myHome/All";

    iot.start_services(addiotnalMQTT);
}
