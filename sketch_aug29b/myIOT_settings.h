myIOT2 iot;
#include <ArduinoJson.h>

#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size
extern void init_buttons();
extern void init_WinSW();

extern void create_SW_instance(JsonDocument &_DOC, uint8_t i);
extern void create_WinSW_instance(JsonDocument &_DOC, uint8_t i);

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[2][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"}; // <----- Verfy file names

void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0)
{
    uint8_t i = 0;
    JsonArray array = DOC[topic];

    if (!array.isNull())
    {
        for (const char *topic : array)
        {
            strlcpy(ch_array[i], topic, MAX_TOPIC_SIZE);
            dest_array[i + shift] = ch_array[i];
            Serial.println(ch_array[i]);
            Serial.flush();
            i++;
        }
    }
}
void updateTopics_flash(JsonDocument &DOC, char ch_array[], const char *dest_array[], const char *topic, u_int8_t i, uint8_t shift = 0)
{
    strlcpy(ch_array, DOC[topic][i], MAX_TOPIC_SIZE);
    dest_array[i + shift] = ch_array;
}
void update_sketch_parameters_flash(JsonDocument &DOC)
{
    RFpin = DOC["RFpin"].as<uint8_t>();
    for (uint8_t i = 0; i < 4; i++)
    {
        RF_keyboardCode[i] = DOC["RF_keyboardCode"][i].as<int>();
    }

    JsonArray entTypes = DOC["entityType"];

    for (uint8_t x = 0; x < entTypes.size(); x++)
    {
        if (entTypes[x] == 0)
        {
            /* create Window instance */
            create_WinSW_instance(DOC, x);
        }
        else if (entTypes[x] == 1)
        {
            /* create Window instance */
            create_SW_instance(DOC, x);
        }
    }
}
void update_Parameters_flash()
{
    StaticJsonDocument<1200> DOC;
    Serial.begin(115200);
    bool readfile_OK = false;

    /* Part A: update filenames of paramter files */
    iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0]));
    // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

    /* Part B: Read from flash, and update myIOT parameters */
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC))
    {
        iot.update_vars_flash_parameters(DOC);
        // serializeJsonPretty(DOC, Serial);
        DOC.clear();
    }
    // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

    // /* Part D: Read Sketch paramters from flash, and update Sketch */
    if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC))
    {
        update_sketch_parameters_flash(DOC);
        // serializeJsonPretty(DOC,Serial);
        DOC.clear();
    }
    // init_WinSW();
    // init_buttons();
    // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

    // /* Part C: Read Topics from flash, and update myIOT Topics */
    readfile_OK = iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC); /* extract topics from flash */
    if (readfile_OK)
    {
        // serializeJsonPretty(DOC,Serial);
        updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
        updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
        updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

        uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
        updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);

        accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
        updateTopics_flash(DOC, buttGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);

        accum_shift += sizeof(buttGroupTopics) / (sizeof(buttGroupTopics[0]));
        for (uint8_t i = 0; i < winEntityCounter; i++)
        {
            updateTopics_flash(DOC, winSW_V[i]->name, iot.topics_sub, "sub_topics_win", i, accum_shift + i);
        }

        accum_shift += winEntityCounter;
        for (uint8_t i = 0; i < swEntityCounter; i++)
        {
            updateTopics_flash(DOC, SW_v[i]->Topic, iot.topics_sub, "sub_topics_SW", i, accum_shift + i);
        }
        DOC.clear();
    }
    else
    {
        /* Tpoics in case of failure getting flash parameters */
        sprintf(topics_gen_pub[0], "myHome/Messages");
        sprintf(topics_gen_pub[1], "myHome/log");
        sprintf(topics_sub[0], "myHome/Cont_Err");
        sprintf(topics_pub[0], "myHome/Cont_Err/State");

        iot.topics_gen_pub[0] = topics_gen_pub[0];
        iot.topics_gen_pub[1] = topics_gen_pub[1];
        iot.topics_pub[0] = topics_pub[0];
        iot.topics_sub[0] = topics_sub[0];
    }
    // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
}

void win_updateState(uint8_t i, uint8_t state) /* Windows State MQTT update */
{
    char t[60];
    char r[5];
    sprintf(t, "%s/State", winSW_V[i]->name);
    sprintf(r, "%d", state);
    iot.pub_noTopic(r, t, true);
}
void butt_updateState(uint8_t i, bool state) /* Button State MQTT update */
{
    char t[60];
    char r[5];
    sprintf(t, "%s/State", SW_v[i]->Topic);
    sprintf(r, "%d", state);
    iot.pub_noTopic(r, t, true);
}
void _gen_WinMSG(uint8_t state, uint8_t reason, uint8_t i)
{
    char msg[100];
    sprintf(msg, "[%s]: [WIN#%d] [%s] turned [%s]", REASONS_TXT[reason], i, winSW_V[i]->name, STATES_TXT[state]);
    iot.pub_msg(msg);
    win_updateState(i, state);
}
void _gen_ButtMSG(uint8_t i, uint8_t type, bool request)
{
    char msg[100];
    sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s]", turnTypes[type], i, SW_v[i]->Topic, request == HIGH ? "ON" : "OFF");
    iot.pub_msg(msg);
    butt_updateState(i, (int)request);
}

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        char msg2[100];
        strcpy(msg, "Status: ");
        if (numW > 0)
        {
            strcat(msg, "[Windows]- ");
            for (uint8_t i = 0; i < numW; i++)
            {
                sprintf(msg2, " Win[#%d][%s] [%d]%s", i, winSW_V[i]->name, winSW_V[i]->get_winState(), i == numSW - 1 ? "" : "; ");
                strcat(msg, msg2);
            }
            iot.pub_msg(msg);
        }
        strcpy(msg, "Status: ");
        if (numSW > 0)
        {
            strcat(msg, "[Switches]- ");
            for (uint8_t i = 0; i < numSW; i++)
            {
                sprintf(msg2, " SW[#%d][%s] [%d]%s", i, SW_v[i]->Topic, _isON(i), i == numSW - 1 ? "" : "; ");
                strcat(msg, msg2);
            }
            iot.pub_msg(msg);
        }
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
    else if (strcmp(winGroupTopics[1], _topic) == 0) /* lockdown */
    {
        if (strcmp(incoming_msg, "true") == 0)
        {
            for (uint8_t i = 0; i < numW; i++)
            {
                iot.pub_msg("Lockdown: Start");
                winSW_V[i]->init_lockdown();
            }
        }
        if (strcmp(incoming_msg, "false") == 0)
        {
            for (uint8_t i = 0; i < numW; i++)
            {
                iot.pub_msg("Lockdown: Released");
                winSW_V[i]->release_lockdown();
            }
        }
    }
    else
    {
        if (strcmp(iot.inline_param[0], EntTypes[0]) == 0 && /* Windows */
            (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0 ||
             strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0 ||
             strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0))
        {
            uint8_t _word = 0;
            if (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0) /* UP */
            {
                _word = UP;
            }
            else if (strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0) /* DOwN */
            {
                _word = DOWN;
            }
            else if (strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0) /* STOP */
            {
                _word = STOP;
            }

            for (uint8_t i = 0; i < numW; i++)
            {
                if (strcmp(_topic, winSW_V[i]->name) == 0) /* SENT FOR A SPECIFIC TOPIC */
                {
                    winSW_V[i]->ext_SW(_word, MQTT);
                    return;
                }
            }
            for (uint8_t i = 0; i < sizeof(winGroupTopics) / sizeof(winGroupTopics[0]); i++) /* OR SENT FOR A GROUP */
            {
                if (strcmp(_topic, winGroupTopics[i]) == 0)
                {
                    for (uint8_t i = 0; i < numW; i++)
                    {
                        winSW_V[i]->ext_SW(_word, MQTT);
                    }
                }
            }
        }
        else if (strcmp(iot.inline_param[0], EntTypes[1]) == 0 &&
                 (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0 ||
                  strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0)) /* MQTT cmd for SW */
        {
            for (uint8_t i = 0; i < numSW; i++)
            {
                if (strcmp(_topic, SW_v[i]->Topic) == 0) /* SENT FOR A SPECIFIC TOPIC */
                {
                    if (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0) /* ON */
                    {
                        _turnON_cb(i, _MQTT);
                        return;
                    }
                    else if (strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0) /* OFF */
                    {
                        _turnOFF_cb(i, _MQTT);
                        return;
                    }
                }
            }
            for (uint8_t i = 0; i < sizeof(buttGroupTopics) / sizeof(buttGroupTopics[0]); i++) /* SENT FOR A GROUP TOPIC */
            {
                if (strcmp(_topic, buttGroupTopics[i]) == 0)
                {
                    for (uint8_t i = 0; i < numSW; i++)
                    {
                        if (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0) /* ON */
                        {
                            _turnON_cb(i, _MQTT);
                        }
                        else if (strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0) /* OFF */
                        {
                            _turnOFF_cb(i, _MQTT);
                        }
                    }
                    return;
                }
            }
        }
    }
}
void startIOTservices()
{
    update_Parameters_flash();
    iot.start_services(addiotnalMQTT);
}