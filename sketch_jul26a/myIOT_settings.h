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
                Serial.println(Lightbut.remainClock(atoi(iot.inline_param[0])));
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
                // Ext_addTime(2, int timeAdd, uint8_t i)
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
