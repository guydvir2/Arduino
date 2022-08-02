extern myIOT2 iot;
extern TurnOnLights lightOutput_0;
extern timeoutButton timeoutButton_0;

extern void Ext_trigger_OFF(uint8_t reason);
extern void Ext_trigger_ON(uint8_t reason, int TO = 0, uint8_t step = 0);
extern void Ext_updatePWM_value(uint8_t reason, uint8_t step);
extern void Ext_addTime(uint8_t reason, int timeAdd);

// const char *INPUTS_ORIGIN[3] = {"Timeout", "Button", "MQTT"};

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

void notifyAdd(int &_add, uint8_t &reason)
{
    char a[100];
    char clk1[25];
    char clk2[25];
    char clk3[25];
    iot.convert_epoch2clock((_add)*60, 0, clk1);
    iot.convert_epoch2clock((timeoutButton_0.timeout + _add) * 60, 0, clk2);
    iot.convert_epoch2clock(timeoutButton_0.remainWatch() + _add, 0, clk3);
    sprintf(a, "%s: Added [%s] ,total [%s] remain [%s]", INPUTS_ORIGIN[reason], clk1, clk2, clk3);
    iot.pub_msg(a);
}
void notifyOFF(uint8_t &reason)
{
    char a[100];
    char clk[25];
    unsigned int remtime = timeoutButton_0.remainWatch();
    iot.convert_epoch2clock(timeoutButton_0.timeout * 60 - remtime, 0, clk);
    sprintf(a, "%s: Switched [Off] after [%s]", INPUTS_ORIGIN[reason], clk);
    if (remtime > 1)
    {
        char b[50];
        iot.convert_epoch2clock(remtime, 0, clk);
        sprintf(b, " Remain [%s]", clk);
        strcat(a, b);
    }
    iot.pub_msg(a);
}
void notifyON(uint8_t &reason)
{
    char a[100];
    char b[50];
    char clk[25];
    iot.convert_epoch2clock(timeoutButton_0.timeout * 60, 0, clk);
    sprintf(a, "%s: Switched [On] for [%s]", INPUTS_ORIGIN[reason], clk);

    if (lightOutput_0.isPWM())
    {
        sprintf(b, " Power [%d/%d]", lightOutput_0.currentStep, lightOutput_0.maxSteps);
        strcat(a, b);
    }

    iot.pub_msg(a);
}
void notifyUpdatePWM(uint8_t &step, uint8_t &reason)
{
    char b[50];
    if (lightOutput_0.isPWM() && timeoutButton_0.getState())
    {
        sprintf(b, "%s: Power update [%d/%d]", INPUTS_ORIGIN[reason], step, lightOutput_0.maxSteps);
    }
    iot.pub_msg(b);
}
void notifyRemain()
{
    char msg[50];
    char s1[20];

    int _rem = timeoutButton_0.remainWatch();
    iot.convert_epoch2clock(_rem, 0, s1);
    sprintf(msg, "MQTT: remain [%s] ", timeoutButton_0.getState() ? s1 : "Off");
    iot.pub_msg(msg);
}
void status_mqtt()
{
    char a[150];
    char b[80];
    char pwmstep[25];
    unsigned int rem = timeoutButton_0.remainWatch();

    sprintf(a, "Status: [%s]", timeoutButton_0.getState() ? "ON" : "OFF");

    if (lightOutput_0.isPWM() && rem > 0)
    {
        sprintf(pwmstep, " Power [%d/%d]", lightOutput_0.currentStep, lightOutput_0.maxSteps);
        strcat(a, pwmstep);
    }

    if (rem > 0)
    {
        char clk[25];
        char clk2[25];
        iot.convert_epoch2clock(rem, 0, clk);
        iot.convert_epoch2clock(timeoutButton_0.timeout * 60 - rem, 0, clk2);
        sprintf(b, " on-Time [%s], Remain[%s]", clk, clk2);
        strcat(a, b);
    }
    iot.pub_msg(a);
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
void update_Parameters_local(){
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
        status_mqtt();
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
    else if (strcmp(iot.inline_param[0], "remain") == 0)
    {
        notifyRemain();
    }
    else if (strcmp(iot.inline_param[0], "off") == 0)
    {
        if (timeoutButton_0.getState())
        {
            Ext_trigger_OFF(2);
        }
    }
    else if (strcmp(iot.inline_param[0], "on") == 0)
    {
        if (timeoutButton_0.getState())
        {
            Ext_trigger_OFF(2);
        }

        if (iot.num_p == 1)
        {
            Ext_trigger_ON(2); /* Default Timeout & default PWM value*/
        }
        else if (iot.num_p == 2)
        {
            Ext_trigger_ON(2, atoi(iot.inline_param[1])); /* define Timeout*/
        }
        else if (iot.num_p == 3)
        {
            Ext_trigger_ON(2, atoi(iot.inline_param[1]), atoi(iot.inline_param[2])); /* define Timeout & PWM*/
        }
    }
    else if (strcmp(iot.inline_param[0], "add") == 0)
    {
        Ext_addTime(2, atoi(iot.inline_param[1]));
    }
    else if (strcmp(iot.inline_param[0], "updatePWM") == 0)
    {
        Ext_updatePWM_value(2, atoi(iot.inline_param[1]));
    }
}
void startIOTservices()
{
    updateTopics_local();
    update_Parameters_local();
    iot.start_services(addiotnalMQTT);
}