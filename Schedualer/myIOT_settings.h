myIOT2 iot;

extern void OFFcmd(uint8_t i = 0, const char *trigger = "timeout");
extern void ONcmd(uint8_t i = 0, uint8_t _TO = 0, const char *trigger = "MQTT");
extern void addWatch(uint8_t i = 0, int _add = 0, const char *trigger = "MQTT");
extern int timeouts[NUM_CHRONOS];
extern Chrono *chronVector[NUM_CHRONOS];

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
            uint8_t m = atoi(iot.inline_param[0]); /* Device num */

            if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (iot.num_p > 2)
                {
                    ONcmd(m, atoi(iot.inline_param[2]));
                }
                else
                {
                    ONcmd(m);
                }
            }
            else if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                char s1[15], s2[7];
                char clk[60], clk2[25], clk3[25];

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
                    OFFcmd(m);
                }
            }
            else if (strcmp(iot.inline_param[1], "add") == 0)
            {
                addWatch(m, atoi(iot.inline_param[2]));
            }
        }
    }
}
void startIOTservices()
{
    iot.useFlashP = true;
    iot.start_services(addiotnalMQTT);
}