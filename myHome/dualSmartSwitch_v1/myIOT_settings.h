myIOT2 iot;
#define MAX_TOPIC_SIZE 40

char topics_sub[4][MAX_TOPIC_SIZE]{};
char topics_pub[3][MAX_TOPIC_SIZE]{};
char topics_gen_pub[3][MAX_TOPIC_SIZE]{};

void create_rem_timeout_msg(uint8_t i, char ret_msg[])
{
    bool sw_is_turned_on_with_timeout = smartSwArray[i]->useTimeout() &&
                                        smartSwArray[i]->telemtryMSG.state == 1;
    bool sw_is_turned_off_before_end_timeout = smartSwArray[i]->useTimeout() &&
                                               smartSwArray[i]->telemtryMSG.state == 0 &&
                                               smartSwArray[i]->get_remain_time() != 0;

    if (sw_is_turned_on_with_timeout || sw_is_turned_off_before_end_timeout)
    {
        char a[15];
        iot.convert_epoch2clock(smartSwArray[i]->get_remain_time() / 1000, 0, a);
        sprintf(ret_msg, ", timeout [%s]", a);
    }
    else
    {
        strcpy(ret_msg, "");
    }
}
void create_pwm_msg(uint8_t i, char ret_msg[], SW_props &props)
{
    bool sw_is_turned_on_pwm = props.PWM && smartSwArray[i]->telemtryMSG.state == 1;

    if (sw_is_turned_on_pwm)
    {
        sprintf(ret_msg, ", Power[%d%%]", smartSwArray[i]->telemtryMSG.pwm);
    }
    else
    {
        strcpy(ret_msg, "");
    }
}
void create_status_msg(uint8_t i, char ret_msg[])
{
    char msg2[30];
    char msg3[30];
    const char *sw_states[] = {"Off", "On"};
    const char *origins[] = {"Switch", "Timeout", "MQTT", "BOOT"};

    SW_props props;
    smartSwArray[i]->get_SW_props(props);

    create_rem_timeout_msg(i, msg2); /* Using timeout ?*/
    create_pwm_msg(i, msg3, props);  /* PWM output ?*/

    sprintf(ret_msg, "[Status]: [%s] is [%s] by [%s]%s%s", props.name, sw_states[smartSwArray[i]->telemtryMSG.state],
            origins[smartSwArray[i]->telemtryMSG.reason], msg3, msg2);
}
void update_MQTT_state(uint8_t i, uint8_t x = 0)
{
    char a[10];
    sprintf(a, "%s", i == 0 ? "off" : "on");
    iot.pub_state(a, x);
}
void smartSW_telemetry2MQTT(uint8_t i)
{
    char msg[150];
    char msg2[30];
    char msg3[30];
    const char *sw_states[] = {"Off", "On"};
    const char *origins[] = {"Switch", "Timeout", "MQTT", "BOOT"};

    SW_props props;
    smartSwArray[i]->get_SW_props(props);

    create_rem_timeout_msg(i, msg2); /* Using timeout ?*/
    create_pwm_msg(i, msg3, props);  /* PWM output ?*/

    sprintf(msg, "[%s]: [%s] turned [%s]%s%s", origins[smartSwArray[i]->telemtryMSG.reason],
            props.name, sw_states[smartSwArray[i]->telemtryMSG.state], msg3, msg2);

    iot.pub_msg(msg);
    update_MQTT_state(smartSwArray[i]->telemtryMSG.state, i);
}
void set_IOT2_Parameters()
{
    iot.useSerial = DEBUG_MODE;
    iot.useFlashP = false;
    iot.noNetwork_reset = 2;
    iot.ignore_boot_msg = false;
}
void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[200];

    if (strcmp(incoming_msg, "status") == 0)
    {
        for (uint8_t i = 0; i < totSW; i++)
        {
            create_status_msg(i, msg);
            iot.pub_msg(msg);
        }
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2: {<i>,on,<power>}, {<i>,timeout,<minutes>,<power>}, off, all_off, entities");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: %s, %s", ver, smartSwArray[0]->ver);
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
            char a[30];
            char clk[15];
            SW_props prop;
            smartSwArray[i]->get_SW_props(prop);
            if (prop.timeout)
            {
                iot.convert_epoch2clock(smartSwArray[i]->get_timeout() / 1000, 0, clk);
                sprintf(a, "%s", clk);
            }
            else
            {
                sprintf(a, "No");
            }

            sprintf(msg, "[Entities]: [#%d][%s], timeout[%s], swType[%d], virtCMD[%s], PWM[%s], output_pin[%d], input_pin[%d], indication_pin[%d] ",
                    prop.id, prop.name, a, prop.type, prop.virtCMD ? "Yes" : "No", prop.PWM ? "Yes" : "No", prop.outpin, prop.inpin, prop.indicpin);
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
                if (smartSwArray[i]->get_remain_time() > 0)
                {
                    smartSwArray[i]->turnOFF_cb(2);
                }
                if (atoi(iot.inline_param[3]) != 0) /* specifing PWR for PWM instance*/
                {
                    smartSwArray[i]->turnON_cb(2, atoi(iot.inline_param[2]) /* timoeut */, atoi(iot.inline_param[3]) /* PWM power */);
                }
                else
                {
                    smartSwArray[i]->turnON_cb(2, atoi(iot.inline_param[2]) /* timoeut */);
                }
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                smartSwArray[i]->turnOFF_cb(2);
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (smartSwArray[i]->get_remain_time() > 0)
                {
                    smartSwArray[i]->turnOFF_cb(2);
                }
                if (atoi(iot.inline_param[2]) != 0) /* specifing PWR for PWM instance*/
                {

                    smartSwArray[i]->turnON_cb(2, 0 /* timeout default value */, atoi(iot.inline_param[2]) /* PWM */);
                }
                else
                {
                    smartSwArray[i]->turnON_cb(2, 0 /* timeout default value */);
                }
            }
        }
        iot.clear_inline_read();
    }
}
void startIOTservices()
{
    iot.start_services(addiotnalMQTT);
}
