myIOT2 iot;
// extern LightButton< > Lightbut;
LightButton<2> Lightbut;

extern void Ext_trigger_OFF(uint8_t reason, uint8_t i);
extern void Ext_trigger_ON(uint8_t reason, int TO = 0, uint8_t step = 0, uint8_t i = 0);
extern void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i = 0);
extern void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i = 0);

// ±±±±±±± Genereal pub topic ±±±±±±±±±
const char *topicLog = "myHome/log";
const char *topicDebug = "myHome/debug";
const char *topicmsg = "myHome/Messages";

// ±±±±±±±±±±±± sub Topics ±±±±±±±±±±±±±±±±±±
const char *topicSub1 = "myHome/alarmMonitor";
const char *topicClient = "myHome/test/Client";
const char *topicAll = "myHome/All";

// ±±±±±±±±±±±±±±±± Client state pub topics ±±±±±±±±±±±±±±±±
const char *topicClient_avail = "myHome/test/Client/Avail";
const char *topicClient_state = "myHome/test/Client/State";

const char *INPUT_ORG[5] = {"Timeout", "Button", "MQTT", "PowerON", "Resume Reboot"};

void notifyAdd(int _add, uint8_t reason, uint8_t i)
{
    char a[100];
    char clk1[25];
    char clk2[25];
    char clk3[25];
    iot.convert_epoch2clock(_add, 0, clk1);
    iot.convert_epoch2clock(Lightbut.get_timeout(i) + _add, 0, clk2);
    iot.convert_epoch2clock(Lightbut.remainClock(i) + _add, 0, clk3);
    sprintf(a, "%s: [%s] Added [%s] ,total [%s] remain [%s]", "INPUT_ORG[reason]", Lightbut.names[i], clk1, clk2, clk3);
    iot.pub_msg(a);
}
void notifyOFF(uint8_t &reason, uint8_t i, int t)
{
    char a[100];
    char clk[25];
    unsigned int remtime = Lightbut.remainClock(i);
    iot.convert_epoch2clock(t, 0, clk);
    sprintf(a, "[%s]: [%s] Switched [Off] after [%s]", INPUT_ORG[reason], Lightbut.names[i], clk);
    if (remtime > 1)
    {
        char b[50];
        iot.convert_epoch2clock(remtime, 0, clk);
        sprintf(b, " Remain [%s]", clk);
        strcat(a, b);
    }
    iot.pub_msg(a);
}
void notifyON(uint8_t &reason, uint8_t i, int t)
{
    char a[100];
    char b[50];
    char clk[25];
    iot.convert_epoch2clock(t, 0, clk);
    sprintf(a, "%s: [%s] [On] for [%s]", INPUT_ORG[reason], Lightbut.names[i], clk);

    if (Lightbut.isPwm(i))
    {
        sprintf(b, " Power [%d/d]", Lightbut.get_counter(i)); //, lightOutputV[i]->maxSteps);
        strcat(a, b);
    }

    iot.pub_msg(a);
}
void notifyState(uint8_t &reason, uint8_t i, int t)
{
    char a[100];
    char b[50];
    char clk[25];
    iot.convert_epoch2clock(t, 0, clk);
    sprintf(a, "[%s]: [%s] [On] for [%s]", INPUT_ORG[reason], Lightbut.names[i], clk);

    if (Lightbut.isPwm(i))
    {
        sprintf(b, " Power [%d/%d]", Lightbut.get_counter(i), Lightbut.get_maxcounter(i));
        strcat(a, b);
    }

    iot.pub_msg(a);
}
void notifyUpdatePWM(uint8_t &step, uint8_t &reason, uint8_t i)
{
    char b[50];
    if (Lightbut.isPwm(i) && Lightbut.getState(i))
    {
        sprintf(b, "%s: [%s] Power update [%d/%d]", INPUT_ORG[reason], Lightbut.names[i], step, 3 /*lightOutputV[i]->maxSteps */);
    }
    iot.pub_msg(b);
}
void notifyRemain(uint8_t i)
{
    char msg[50];
    char s1[20];

    int _rem = Lightbut.remainClock(i);
    iot.convert_epoch2clock(_rem, 0, s1);
    sprintf(msg, "MQTT: [%s] remain [%s] ", Lightbut.names[i], Lightbut.getState(i) ? s1 : "Off");
    iot.pub_msg(msg);
}

void updateTopics_local()
{
    iot.topics_gen_pub[0] = topicmsg;
    iot.topics_gen_pub[1] = topicLog;
    iot.topics_gen_pub[2] = topicDebug;

    iot.topics_pub[0] = topicClient_avail;
    iot.topics_pub[1] = topicClient_state;

    iot.topics_sub[0] = topicClient;
    iot.topics_sub[1] = topicAll;
    iot.topics_sub[2] = topicSub1;
}
void update_Parameters_local()
{
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useSerial = true;
    iot.useResetKeeper = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useFlashP = false;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;
}
void addiotnalMQTT(char *incoming_msg, char *_topic)
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
    else
    {
        if (iot.num_p > 1)
        {
            if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                notifyRemain(atoi(iot.inline_param[0]));
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                if (Lightbut.getState(atoi(iot.inline_param[0])))
                {
                    Ext_trigger_OFF(2, atoi(iot.inline_param[0]));
                }
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (Lightbut.getState(atoi(iot.inline_param[0]))) /* turn off if it is ON */
                {
                    Ext_trigger_OFF(2, atoi(iot.inline_param[0]));
                }

                if (iot.num_p == 2)
                {
                    Ext_trigger_ON(2, 0, 0, atoi(iot.inline_param[0])); /* Default Timeout & default PWM value*/
                }
                else if (iot.num_p == 3)
                {
                    Ext_trigger_ON(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0])); /* define Timeout*/
                }
                else if (iot.num_p == 4)
                {
                    Ext_trigger_ON(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[3]), atoi(iot.inline_param[0])); /* define Timeout & PWM*/
                }
            }
            else if (strcmp(iot.inline_param[1], "add") == 0)
            {
                Ext_addTime(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0]));
            }
            else if (strcmp(iot.inline_param[1], "updatePWM") == 0)
            {
                Ext_updatePWM_value(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0]));
            }
        }
    }
}
void startIOTservices()
{
    updateTopics_local();
    update_Parameters_local();
    iot.start_services(addiotnalMQTT);
}
