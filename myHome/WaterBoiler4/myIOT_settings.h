#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

// char topics_sub[3][MAX_TOPIC_SIZE];
// char topics_pub[3][MAX_TOPIC_SIZE];
// char topics_gen_pub[3][MAX_TOPIC_SIZE];

void get_status(char *outputmsg)
{
    char t1[100];
    char t2[50];
    char t3[50];

    if (SWitch.get_remain_time() > 0)
    {
        iot.convert_epoch2clock(SWitch.get_remain_time(), 0, t2);
        iot.convert_epoch2clock(SWitch.get_timeout(), 0, t3);
        sprintf(t1, "timeLeft[%s], total[%s]", t2, t3);
    }
    else
    {
        sprintf(t1, " ");
    }
    sprintf(t2, "[%s] %s", SWitch.get_SWstate() ? "ON" : "OFF", t1);
    sprintf(outputmsg, "Status: %s", t2);
}
void addiotnalMQTT(char *income_msg, char *topic)
{
    char msg_MQTT[150];

    if (strcmp(income_msg, "status") == 0)
    {
        get_status(msg_MQTT);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "ver2") == 0)
    {
        sprintf(msg_MQTT, "ver #2: [%s], timeoutSw[%s]", VEr, SWitch.ver);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "help2") == 0)
    {
        sprintf(msg_MQTT, "Help2: Commands #1 - [on, off, remain, {addTO,minutes}, {timeout,minutes}]");
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "off") == 0)
    {
        SWitch.turnOFF_cb(EXT_0);
    }
    else if (strcmp(income_msg, "on") == 0)
    {
        SWitch.turnON_cb(EXT_0);
    }
    else if (strcmp(income_msg, "remain") == 0)
    {
        char s1[15];
        char clk[60];
        if (SWitch.get_remain_time() > 0)
        {
            iot.convert_epoch2clock(SWitch.get_remain_time(), 0, s1);
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
                SWitch.turnON_cb(2, atoi(iot.inline_param[1]));
                iot.pub_msg(iot.inline_param[1]);
            }
            else if (strcmp(iot.inline_param[0], "addTO") == 0)
            {
                // TOswitch.add_TO(atoi(iot.inline_param[1]), 2);
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
    iot.topics_gen_pub[2] = "myHome/debug";

    iot.topics_pub[0] = "myHome/WaterBoiler";
    iot.topics_pub[0] = "myHome/WaterBoiler/Avail";
    iot.topics_pub[1] = "myHome/WaterBoiler/State";
    iot.topics_sub[0] = "myHome/WaterBoiler";
    iot.topics_sub[1] = "myHome/All";

    iot.start_services(addiotnalMQTT);
}
