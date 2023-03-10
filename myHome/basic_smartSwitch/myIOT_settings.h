myIOT2 iot;
#define MAX_TOPIC_SIZE 40

char topics_sub[4][MAX_TOPIC_SIZE]{};
char topics_pub[3][MAX_TOPIC_SIZE]{};
char topics_gen_pub[3][MAX_TOPIC_SIZE]{};

void update_MQTT_state(uint8_t i, uint8_t x = 0)
{
    char a[5];
    sprintf(a, "%s", i == 0 ? "off" : "on");
    iot.pub_state(a, x);
}
void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];

    if (strcmp(incoming_msg, "status") == 0)
    {
        char a[30];
        char b[15];
        for (uint8_t i = 0; i < totSW; i++)
        {
            if (smartSwArray[i]->get_SWstate() == 1 && smartSwArray[i]->useTimeout() && smartSwArray[i]->get_remain_time() != 0)
            {
                iot.convert_epoch2clock(smartSwArray[i]->get_remain_time() / 1000, 0, b);
                sprintf(a, ", timeout [%s]", b);
            }
            else
            {
                strcpy(a, "");
            }
            if (totSW > 1)
            {
                SW_props prop;
                smartSwArray[i]->get_SW_props(prop);
                sprintf(msg, "[status]: [%s] turned [%s]%s", prop.name, smartSwArray[i]->get_SWstate() == 1 ? "On" : "Off", a);
            }
            else
            {
                sprintf(msg, "[status]: turned [%s]%s", smartSwArray[i]->get_SWstate() == 1 ? "On" : "Off", a);
            }
            iot.pub_msg(msg);
        }
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2: on, off, all_off,entities, {timeout,MIN}");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2:");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "all_off") == 0)
    {
        for (uint8_t i = 0; i < totSW; i++)
        {
            smartSwArray[i]->turnOFF_cb(2);
        }
    }
    else if (strcmp(incoming_msg, "entities") == 0)
    {
        for (uint8_t i = 0; i < totSW; i++)
        {
            SW_props prop;
            smartSwArray[i]->get_SW_props(prop);

            sprintf(msg, "[Entities]: #%d name[%s], timeout[%s], swType[%d], virtCMD[%s], PWM[%s], output_pin[%d], input_pin[%d], indication_pin[%d]",
                    prop.id, prop.name, prop.timeout ? "Yes" : "No", prop.type, prop.virtCMD ? "Yes" : "No", prop.PWM ? "Yes" : "No",
                    prop.outpin, prop.inpin, prop.indicpin);
            iot.pub_msg(msg);
        }
    }
    else
    {
        if (iot.num_p > 1)
        {
            uint8_t i = atoi(iot.inline_param[0]);
            if (strcmp(iot.inline_param[1], "timeout") == 0)
            {
                smartSwArray[i]->turnON_cb(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[3]));
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                smartSwArray[i]->turnOFF_cb(2);
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                smartSwArray[i]->turnON_cb(2, NULL, atoi(iot.inline_param[2]));
            }
        }
    }
}
void startIOTservices()
{
    iot.start_services(addiotnalMQTT);
}
