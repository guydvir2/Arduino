#define numSW 2

myIOT2 iot;
LightButton<numSW> Lightbut;

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

void notifyState(uint8_t &reason, uint8_t i, int t)
{
    char a[100];
    char b[50];
    char clk[25];
    iot.convert_epoch2clock(t, 0, clk);
    sprintf(a, "[%s]: [%s] [%s] %s [%s]", INPUT_ORG[reason], Lightbut.names[i], Lightbut.getState(i) ? "On" : "Off", Lightbut.getState(i) ? "for" : "after", clk);

    if (Lightbut.isPwm(i) && Lightbut.getState(i))
    {
        sprintf(b, " Power [%d/%d]", Lightbut.get_counter(i), Lightbut.get_maxcounter(i));
        strcat(a, b);
    }

    iot.pub_msg(a);
}
void notifyUpdatePWM(uint8_t step, uint8_t reason, uint8_t i)
{
    char b[50];
    if (Lightbut.isPwm(i) && Lightbut.getState(i))
    {
        sprintf(b, "%s: [%s] Power update [%d/%d]", INPUT_ORG[reason], Lightbut.names[i], step, Lightbut.get_maxcounter(i));
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
        for (uint8_t i = 0; i < numSW; i++)
        {
            sprintf(msg, "[Status]: [%s] [%s]", Lightbut.names[i], Lightbut.getState(i) ? "On" : "Off");
            if (Lightbut.getState(i))
            {
                char t2[100];
                char s1[20];
                char s2[10];
                int _rem = Lightbut.remainClock(i);
                iot.convert_epoch2clock(_rem, 0, s1);
                if (Lightbut.isPwm(i))
                {
                    sprintf(s2, "%d/%d", Lightbut.get_counter(i), Lightbut.get_maxcounter(i));
                }
                sprintf(t2, " Remain: [%s] PWM[%s]", s1, Lightbut.isPwm(i) ? s2 : "NO");
                strcat(msg, t2);
            }
            iot.pub_msg(msg);
        }
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
        if (iot.num_p > 1 && atoi(iot.inline_param[0]) < numSW)
        {
            if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                notifyRemain(atoi(iot.inline_param[0]));
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                if (Lightbut.getState(atoi(iot.inline_param[0])))
                {
                    Lightbut.TurnOFF(2, atoi(iot.inline_param[0]));
                }
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (Lightbut.getState(atoi(iot.inline_param[0]))) /* turn off if it is ON */
                {
                    Lightbut.TurnOFF(2, atoi(iot.inline_param[0]));
                }

                if (iot.num_p == 2)
                {
                    Lightbut.TurnON(0, 2, Lightbut.get_defcounter(atoi(iot.inline_param[0])), atoi(iot.inline_param[0])); /* Default Timeout & default PWM value*/
                }
                else if (iot.num_p == 3)
                {
                    Lightbut.TurnON(atoi(iot.inline_param[2]), 2, Lightbut.get_defcounter(atoi(iot.inline_param[0])), atoi(iot.inline_param[0])); /* define Timeout*/
                }
                else if (iot.num_p == 4)
                {
                    Lightbut.TurnON(atoi(iot.inline_param[2]), 2, atoi(iot.inline_param[3]), atoi(iot.inline_param[0])); /* define Timeout & PWM*/
                }
            }
            else if (strcmp(iot.inline_param[1], "add") == 0)
            {
                Lightbut.addClock(conv2Minute(atoi(iot.inline_param[2])), 2, atoi(iot.inline_param[0]));
            }
            else if (strcmp(iot.inline_param[1], "updatePWM") == 0)
            {
                if (Lightbut.getState(atoi(iot.inline_param[0])) == true) /* if already ON */
                {
                    Lightbut.set_PWM(atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                    notifyUpdatePWM(atoi(iot.inline_param[2]), 2, atoi(iot.inline_param[0]));
                }
                else
                {
                    Lightbut.TurnON(0, 2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0]));
                }
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
