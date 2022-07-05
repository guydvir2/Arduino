extern myIOT2 iot;
extern TurnOnLights led0;
extern timeoutButton TO_Button_0;

extern void OFF_MQTT(uint8_t reason);
extern void ON_MQTT(uint8_t reason, int TO = 0, uint8_t step = 0);
extern void updatePWM(uint8_t reason, uint8_t step);
extern void addTime_MQTT(uint8_t reason, int timeAdd);

const char *INPUTS_ORIGIN[] = {"Timeout", "Button", "MQTT"};

void notifyAdd(int &_add, uint8_t &reason)
{
    char a[100];
    char clk1[25];
    char clk2[25];
    char clk3[25];
    iot.convert_epoch2clock((_add) * 60, 0, clk1);
    iot.convert_epoch2clock((TO_Button_0.timeout+ _add) * 60, 0, clk2);
    iot.convert_epoch2clock(TO_Button_0.remainWatch()+_add, 0, clk3);
    sprintf(a, "%s: Added [%s] ,total [%s] remain [%s]", INPUTS_ORIGIN[reason],clk1,clk2,clk3);
    iot.pub_msg(a);
}
void notifyOFF(uint8_t &reason)
{
    char a[100];
    char clk[25];
    unsigned int remtime = TO_Button_0.remainWatch();
    iot.convert_epoch2clock(TO_Button_0.timeout * 60 - remtime, 0, clk);
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
    iot.convert_epoch2clock(TO_Button_0.timeout * 60, 0, clk);
    sprintf(a, "%s: Switched [On] for [%s]", INPUTS_ORIGIN[reason], clk);

    if (led0.isPWM())
    {
        sprintf(b, " Power [%d/%d]", led0.currentStep, led0.maxSteps);
        strcat(a, b);
    }

    iot.pub_msg(a);
}
void notifyUpdatePWM(uint8_t &step, uint8_t &reason)
{
    char b[50];
    if (led0.isPWM() && TO_Button_0.getState())
    {
        sprintf(b, "%s: Power update [%d/%d]", INPUTS_ORIGIN[reason], step, led0.maxSteps);
    }
    iot.pub_msg(b);
}
void notifyRemain(char *msg)
{
    char s1[20];

    int _rem = TO_Button_0.remainWatch();
    iot.convert_epoch2clock(_rem, 0, s1);
    sprintf(msg, "MQTT: remain [%s] ", TO_Button_0.getState() ? s1 : "Off");
    iot.pub_msg(msg);
}

void status_mqtt(char *a)
{
    char b[50];
    char pwmstep[25];
    unsigned int rem = TO_Button_0.remainWatch();

    sprintf(a, "Status: [%s]", TO_Button_0.getState() ? "ON" : "OFF");

    if (led0.isPWM() && rem > 0)
    {
        sprintf(pwmstep, " Power [%d/%d]", led0.currentStep, led0.maxSteps);
        strcat(a, pwmstep);
    }

    if (rem > 0)
    {
        char clk[25];
        char clk2[25];
        iot.convert_epoch2clock(rem, 0, clk);
        iot.convert_epoch2clock(TO_Button_0.timeout * 60 - rem, 0, clk2);
        sprintf(b, " on-Time [%s], Remain[%s]", clk, clk2);
        strcat(a, b);
    }
    iot.pub_msg(a);
}
void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        status_mqtt(msg);
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
        notifyRemain(msg);
    }
    else if (strcmp(iot.inline_param[0], "off") == 0)
    {
        if (TO_Button_0.getState())
        {
            OFF_MQTT(2);
        }
    }
    else if (strcmp(iot.inline_param[0], "on") == 0)
    {
        if (TO_Button_0.getState())
        {
            OFF_MQTT(2);
        }

        if (iot.num_p == 1)
        {
            ON_MQTT(2); /* Default Timeout & default PWM value*/
        }
        else if (iot.num_p == 2)
        {
            ON_MQTT(2, atoi(iot.inline_param[1])); /* define Timeout*/
        }
        else if (iot.num_p == 3)
        {
            ON_MQTT(2, atoi(iot.inline_param[1]), atoi(iot.inline_param[2])); /* define Timeout & PWM*/
        }
    }
    else if (strcmp(iot.inline_param[0], "add") == 0)
    {
        addTime_MQTT(2, atoi(iot.inline_param[1]));
    }
    else if (strcmp(iot.inline_param[0], "updatePWM") == 0)
    {
        updatePWM(2, atoi(iot.inline_param[1]));
    }
}
void startIOTservices()
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

    iot.TOPICS_JSON["pub_gen_topics"][0] = "myHome/Messages";
    iot.TOPICS_JSON["pub_gen_topics"][1] = "myHome/log";
    iot.TOPICS_JSON["pub_gen_topics"][2] = "myHome/debug";

    iot.TOPICS_JSON["pub_topics"][0] = "myHome/group/client/Avail";
    iot.TOPICS_JSON["pub_topics"][1] = "myHome/group/client/State";

    iot.TOPICS_JSON["sub_topics"][0] = "myHome/group/client";
    iot.TOPICS_JSON["sub_topics"][1] = "myHome/All";

    iot.TOPICS_JSON["sub_data_topics"][0] = "myHome/device1";
    iot.TOPICS_JSON["sub_data_topics"][1] = "myHome/alarmMonitor";

    iot.start_services(addiotnalMQTT);
}
