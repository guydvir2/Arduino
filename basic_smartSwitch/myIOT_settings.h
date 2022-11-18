myIOT2 iot;
#define MAX_TOPIC_SIZE 40

extern void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t pwm_pwr,
                         uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot);

char topics_sub[3][MAX_TOPIC_SIZE]{};
char topics_pub[3][MAX_TOPIC_SIZE]{};
char topics_gen_pub[3][MAX_TOPIC_SIZE]{};

const char *MCUtypeIO[] = {"/io_fail.json", "/io_SONOFF_S26.json", "/io_SONOFF_mini.json", "/io_MCU.json"};
const char *parameterFiles[] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json", MCUtypeIO[0]}; /* MCUtypeIO value is updated from flash */

void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t ar_size)
{
    for (uint8_t i = 0; i < ar_size; i++)
    {
        strlcpy(ch_array[i], DOC[topic][i] | defaulttopic, MAX_TOPIC_SIZE);
        dest_array[i] = ch_array[i];
    }
}
void update_Parameters_flash()
{
    StaticJsonDocument<850> DOC;

    const char *SWname{};
    uint8_t numS{};
    uint8_t mcuType{};
    uint8_t butType{};
    uint8_t input_pin{};
    uint8_t indic_pin{};
    uint8_t output_pin{};
    uint8_t pwm_pwr{};
    int timeout_duration{};
    bool useclkdown = false;
    bool indic_on = false;
    bool onatboot = false;

    /* Part A: update filenames of paramter files */
    iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0]));

    /* Part B: Read from flash, and update myIOT parameters */
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC))
    {
        iot.update_vars_flash_parameters(DOC);
    }
    DOC.clear();

    /* Part C: Read Topics from flash, and update myIOT Topics */
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC))
    { /* extract topics from flash */
        updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics", "myHome/Messages", sizeof(topics_gen_pub) / (sizeof(topics_gen_pub[0])));
        updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics", "myHome/log", sizeof(topics_pub) / (sizeof(topics_pub[0])));
        updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics", "myHome/log", sizeof(topics_sub) / (sizeof(topics_sub[0])));
    }
    DOC.clear();

    uint8_t x = 0;

    while (x < MAX_SW)
    {
        /* Part D: Read Sketch paramters from flash, and update Sketch */
        if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC))
        {
            numS = DOC["numSW"].as<uint8_t>();
            mcuType = DOC["MCUtype"].as<uint8_t>();
            useclkdown = DOC["useLockdown"].as<bool>();

            onatboot = DOC["ONatBoot"][x].as<bool>();
            butType = DOC["SWtype"][x].as<uint8_t>();
            SWname = DOC["name"][x].as<const char *>();
            timeout_duration = DOC["Timeout"][x].as<int>();
            iot.parameter_filenames[3] = MCUtypeIO[mcuType];
            // parameterFiles[3] = MCUtypeIO[mcuType];
            // iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0]));
        }
        DOC.clear();

        /* Part E: Read MCU IOs paramters from flash, and update Sketch */
        if (iot.extract_JSON_from_flash(MCUtypeIO[mcuType], DOC))
        {
            input_pin = DOC["inputPin"][x].as<uint8_t>();
            output_pin = DOC["relayPin"][x].as<uint8_t>();
            indic_pin = DOC["indicLED"][x].as<uint8_t>() | 255;
            indic_on = DOC["indic_on"][x].as<bool>() | false;
            pwm_pwr = DOC["PWM_pwr"][x].as<uint8_t>() | 0;
        }

        smartSW_defs(x, SWname, butType, output_pin, pwm_pwr, input_pin, indic_pin, useclkdown, timeout_duration, indic_on, onatboot);
        DOC.clear();

        if (numS == 1)
        {
            x = MAX_SW;
        }
        else
        {
            x++;
        }
    }
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
            sprintf(msg, "[status]: [%d] turned [%s]%s", i, smartSwArray[i]->get_SWstate() == 1 ? "On" : "Off", a);
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
                smartSwArray[i]->turnON_cb(2, 60 * atoi(iot.inline_param[2]));
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                smartSwArray[i]->turnOFF_cb(2);
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                smartSwArray[i]->turnON_cb(2);
            }
        }
    }
}
void startIOTservices()
{
    update_Parameters_flash();
    iot.start_services(addiotnalMQTT);
}
