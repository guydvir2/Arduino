extern myIOT2 iot;
extern homeCtl controller;

extern char topics_sub[3][MAX_TOPIC_SIZE];
extern char topics_pub[2][MAX_TOPIC_SIZE];
extern char topics_gen_pub[3][MAX_TOPIC_SIZE];
extern char winGroupTopics[2][MAX_TOPIC_SIZE];
extern char SwGroupTopics[3][MAX_TOPIC_SIZE];

extern bool getPins_manual(JsonDocument &DOC);

/* ±±±±±±±±± Filenames and directories for each controller ±±±±±±±±±±± */

char parameterFiles[4][30];

void construct_filenames(uint8_t i = 0)
{
    uint8_t tot_num_files = 4;
    uint8_t num_common_files = 2;

    const char *dirs[] = {"Fail", "Cont_A", "Cont_B", "Cont_C", "Cont_D", "Cont_test"};
#if defined(ESP32)
    const char *FileNames_common[2] = {"myIOT_param.json", "HardwareESP32.json"};
#elif defined(ESP8266)
    const char *FileNames_common[2] = {"myIOT_param.json", "HardwareESP8266.json"};
#endif
    const char *FileNames_dedicated[2] = {"myIOT2_topics.json", "sketch_param.json"};

    for (uint8_t x = 0; x < tot_num_files; x++)
    {
        if (x < num_common_files)
        {
            sprintf(parameterFiles[x], "/%s", FileNames_common[x]);
        }
        else
        {
            sprintf(parameterFiles[x], "/%s/%s", dirs[i], FileNames_dedicated[x - num_common_files]);
        }
        iot.parameter_filenames[x] = parameterFiles[x];
    }
}

/* ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±± */

void _updateTopics_flash(JsonDocument &DOC, char ch_array[], const char *dest_array[], const char *topic, uint8_t i, uint8_t shift = 0) /* update ent Name property */
{
    strlcpy(ch_array, DOC[topic][i].as<const char *>(), MAX_TOPIC_SIZE);
    dest_array[i + shift] = ch_array;
}
void _updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0) /* update local Topic array */
{
    uint8_t i = 0;
    JsonArray array = DOC[topic];

    if (!array.isNull())
    {
        for (const char *topic : array)
        {
            strlcpy(ch_array[i], topic, MAX_TOPIC_SIZE);
            dest_array[i + shift] = ch_array[i];
            i++;
        }
    }
}
void readTopics()
{
    StaticJsonDocument<1200> DOC;
#if MAN_MODE && LOCAL_PARAM
    DeserializationError error0 = deserializeJson(DOC, topics);
    if (!error0)
    {
#else
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC)) /* extract topics from flash */
    {
#endif
        _updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
        _updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
        _updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

        uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));

        //  >>>>> Update Windows Group Topics >>>>>>>>>
        if (controller.get_ent_counter(WIN_ENT) > 0)
        {
            _updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);
            accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
        }

        //  >>>>> Update Switch Group Topics >>>>>>>>>
        if (controller.get_ent_counter(SW_ENT) > 0)
        {
            _updateTopics_flash(DOC, SwGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);
            accum_shift += sizeof(SwGroupTopics) / (sizeof(SwGroupTopics[0]));
        }

        //  >>>>> Update Windows Topics >>>>>>>>>
        for (uint8_t i = 0; i < controller.get_ent_counter(WIN_ENT); i++)
        {
            Win_props win_prop;
            controller.get_entity_prop(WIN_ENT, i, win_prop);
            if (!win_prop.virtCMD)
            {
                _updateTopics_flash(DOC, win_prop.name, iot.topics_sub, "sub_topics_win", i, accum_shift + i);
            }
        }
        accum_shift += controller.get_ent_counter(WIN_ENT);

        //  >>>>> Update Switch Topics >>>>>>>>>
        for (uint8_t i = 0; i < controller.get_ent_counter(SW_ENT); i++)
        {
            SW_props sw_prop;
            controller.get_entity_prop(SW_ENT, i, sw_prop);
            if (sw_prop.virtCMD == false)
            {
                _updateTopics_flash(DOC, sw_prop.name, iot.topics_sub, "sub_topics_SW", i, accum_shift + i);
            }
        }
    }
}

void _update_HWpins_flash(JsonDocument &DOC, uint8_t in[], uint8_t out[], int RF[], uint8_t &size_in, uint8_t &size_out, uint8_t &size_RF, uint8_t &RF_pin)
{
    RF_pin = DOC["RFpin"];
    size_in = DOC["inputPins"].size() | 1;
    size_out = DOC["relayPins"].size() | 1;
    size_RF = DOC["RF_keyboardCode"].size() | 1;

    for (uint8_t i = 0; i < size_RF; i++)
    {
        RF[i] = DOC["RF_keyboardCode"][i].as<int>() | 0;
    }
    for (uint8_t i = 0; i < size_in; i++)
    {
        in[i] = DOC["inputPins"][i] | 255;
    }
    for (uint8_t i = 0; i < size_out; i++)
    {
        out[i] = DOC["relayPins"][i] | 255;
    }
}
void read_Hardware(JsonDocument &DOC, uint8_t _inpins[], uint8_t _outpins[], int _RFcodes[], uint8_t &in, uint8_t &out, uint8_t &RF, uint8_t RFp)
{
#if MAN_MODE && LOCAL_PARAM
    if (getPins_manual(DOC))
    {
#else
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC))
    {
#endif
        _update_HWpins_flash(DOC, _inpins, _outpins, _RFcodes, in, out, RF, RFp);
    }
}

void read_IOT()
{
    StaticJsonDocument<400> DOC;
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC))
    {
        iot.update_vars_flash_parameters(DOC);
    }
}

void _create_entities(JsonDocument &DOC, uint8_t input_pins[], uint8_t output_pins[], int RF_keyboardCode[], uint8_t &size_in, uint8_t &size_out, uint8_t &size_RF, uint8_t RF)
{
    JsonArray entTypes = DOC["entityType"];
    uint8_t win_ents = 0;
    uint8_t sw_ents = 0;

    controller.set_RF(RF);
    controller.set_RFch(RF_keyboardCode, size_RF);
    controller.set_inputs(input_pins, size_in);
    controller.set_outputs(output_pins, size_out);

    for (uint8_t x = 0; x < entTypes.size(); x++)
    {
        if (entTypes[x].as<uint8_t>() == WIN_ENT)
        {
            controller.create_Win(DOC["Winname"][win_ents].as<const char *>(), DOC["WinvirtCMD"][win_ents].as<bool>(), DOC["WextInputs"][win_ents].as<uint8_t>());
            win_ents++;
        }
        else if (entTypes[x].as<uint8_t>() == SW_ENT)
        {
            controller.create_SW(DOC["SWname"][sw_ents].as<const char *>(), DOC["SW_buttonTypes"][sw_ents].as<uint8_t>(),
                                 DOC["SWvirtCMD"][sw_ents].as<bool>(), DOC["SW_timeout"][sw_ents].as<uint8_t>(), DOC["RF_2entity"][sw_ents].as<uint8_t>());
            sw_ents++;
        }
    }
}
void read_Entities()
{
    StaticJsonDocument<1200> DOC;
    
    int _RFcodes[10]{};
    uint8_t _inpins[12]{};
    uint8_t _outpins[8]{};
    uint8_t RF_p = 0;
    uint8_t actual_inpins_saved = 0;
    uint8_t actual_outpins_saved = 0;
    uint8_t actual_RFcodes_saved = 0;

    read_Hardware(DOC, _inpins, _outpins, _RFcodes, actual_inpins_saved, actual_outpins_saved, actual_RFcodes_saved, RF_p);

#if MAN_MODE && LOCAL_PARAM
    DeserializationError error0 = deserializeJson(DOC, cont_params);
    if (!error0)
    {
#else
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[3], DOC))
    {
#endif
        _create_entities(DOC, _inpins, _outpins, _RFcodes, actual_inpins_saved, actual_outpins_saved, actual_RFcodes_saved, RF_p);
    }
}

void read_all_parameters()
{
    construct_filenames();
    read_IOT();
    read_Entities();
    readTopics();
}