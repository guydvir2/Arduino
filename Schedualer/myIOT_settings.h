void notifyAdd(uint8_t &i, int &_add, const char *trigger)
{
    char a[50];
    sprintf(a, "%s: [%s] Added [%d] seconds, total [%d] seconds", trigger, sw_names[i], _add, timeouts[i]);
    iot.pub_msg(a);
}
void notifyOFF(uint8_t &i, int &_elapsed, const char *trigger)
{
    char a[50];
    sprintf(a, "%s: [%s] Switched [Off] after [%d] seconds", trigger, sw_names[i], _elapsed);
    iot.pub_msg(a);
}
void notifyON(uint8_t &i, const char *trigger)
{
    char a[50];
    sprintf(a, "%s: [%s] Switched [On] for [%d] seconds", trigger, sw_names[i], timeouts[i]);
    iot.pub_msg(a);
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
            uint8_t m = atoi(iot.inline_param[0]); /* Device num temp variable */

            if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (iot.num_p > 3)
                {
                    ONcmd(m, atoi(iot.inline_param[2]), SRCS[MQTT_CMD], atoi(iot.inline_param[3])); /* PWM */
                }
                else if (iot.num_p > 2)
                {
                    ONcmd(m, atoi(iot.inline_param[2]), SRCS[MQTT_CMD]); /* Custom Timeout*/
                }
                else
                {
                    ONcmd(m, 0, SRCS[MQTT_CMD]); /* Default Timeout */
                }
            }
            else if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                char s1[15];
                char clk[60];

                if (chronVector[m]->isRunning())
                {
                    int _rem = timeouts[m] - chronVector[m]->elapsed();
                    iot.convert_epoch2clock(_rem, 0, s1);
                    sprintf(clk, "MQTT: remain [%s] ", s1);
                    iot.pub_msg(clk);
                }
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                if (chronVector[m]->isRunning())
                {
                    OFFcmd(m, SRCS[MQTT_CMD]);
                }
            }
            else if (strcmp(iot.inline_param[1], "add") == 0)
            {
                addWatch(m, atoi(iot.inline_param[2]), SRCS[MQTT_CMD]);
            }
        }
    }
}
void startIOTservices()
{
#if FlashParameters == 1
    iot.useFlashP = true;
#else
    iot.useFlashP = false;

    iot.TOPICS_JSON["pub_gen_topics"][0] = "myHome/Messages";
    iot.TOPICS_JSON["pub_gen_topics"][1] = "myHome/log";

    iot.TOPICS_JSON["pub_topics"][0] = "myHome/intlights/test/Avail";
    iot.TOPICS_JSON["pub_topics"][1] = "myHome/intlights/test/State";

    iot.TOPICS_JSON["sub_topics"][0] = "myHome/intlights/test";
    iot.TOPICS_JSON["sub_topics"][1] = "myHome/All";

    iot.TOPICS_JSON["sub_data_topics"][0] = "myHome/device1";
    iot.TOPICS_JSON["sub_data_topics"][1] = "myHome/alarmMonitor";
#endif

    iot.start_services(addiotnalMQTT);
}