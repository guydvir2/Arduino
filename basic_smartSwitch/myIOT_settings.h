myIOT2 iot;
#define MAX_TOPIC_SIZE 40

extern smartSwitch smartSW;
extern void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t input_pin,
                         uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot);

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[3][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

char *MCUtypeIO[] = {"/io_fail.json", "/io_SONOFF_S26.json","/io_SONOFF_mini.json"};
char *parameterFiles[] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"};

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
    uint8_t mcuType{};
    uint8_t butType{};
    uint8_t input_pin{};
    uint8_t indic_pin{};
    uint8_t output_pin{};
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

    /* Part D: Read Sketch paramters from flash, and update Sketch */
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC))
    {
        butType = DOC["SWtype"].as<uint8_t>();
        mcuType = DOC["MCUtype"].as<uint8_t>();
        timeout_duration = DOC["Timeout"].as<int>();
        SWname = DOC["name"].as<const char *>();
        onatboot = DOC["ONatBoot"].as<bool>();
        useclkdown = DOC["useLockdown"].as<bool>();
    }
    DOC.clear();

    /* Part E: Read MCU IOs paramters from flash, and update Sketch */
    if (iot.extract_JSON_from_flash(MCUtypeIO[mcuType], DOC))
    {
        input_pin = DOC["inputPin"].as<uint8_t>();
        output_pin = DOC["relayPin"].as<uint8_t>();
        indic_pin = DOC["indicLED"].as<uint8_t>();
        indic_on = DOC["indic_on"].as<bool>();
    }

    smartSW_defs(0, SWname, butType, output_pin, input_pin, indic_pin, useclkdown, timeout_duration, indic_on, onatboot);
    DOC.clear();
}

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        char a[30];
        char b[15];
        if (smartSW.get_SWstate() == 1 && smartSW.useTimeout() && smartSW.get_remain_time() != 0)
        {
            iot.convert_epoch2clock(smartSW.get_remain_time() / 1000, 0, b);
            sprintf(a, ", timeout [%s]", b);
        }
        else
        {
            strcpy(a, "");
        }
        sprintf(msg, "status: turned [%s]%s", smartSW.get_SWstate() == 1 ? "On" : "Off", a);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2: on, off, {timeout,MIN}");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2:");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "off") == 0)
    {
        smartSW.turnOFF_cb(2);
    }
    else if (strcmp(incoming_msg, "on") == 0)
    {
        smartSW.turnON_cb(2);
    }
    else
    {
        if (iot.num_p > 1 && strcmp(iot.inline_param[0], "timeout") == 0)
        {
            smartSW.turnON_cb(2, atoi(iot.inline_param[1]));
        }
    }
}
void startIOTservices()
{
    update_Parameters_flash();
    iot.start_services(addiotnalMQTT);
}
