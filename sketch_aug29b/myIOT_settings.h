myIOT2 iot;

#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[2][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

char winGroupTopics[4][MAX_TOPIC_SIZE];
char buttGroupTopics[3][MAX_TOPIC_SIZE];

char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"}; // <----- Verfy file names

void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t ar_size, uint8_t shift = 0)
{
    JsonArray array = DOC[topic].to<JsonArray>();
    Serial.println(array.size());
    for (uint8_t i = 0; i < ar_size; i++)
    {
        Serial.println(DOC[topic][i].isNull());
        strlcpy(ch_array[i], DOC[topic][i] | defaulttopic, MAX_TOPIC_SIZE);
        dest_array[i + shift] = ch_array[i];
    }
}
void updateTopics_flash(JsonDocument &DOC, char ch_array[], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t i, uint8_t shift = 0)
{
    strlcpy(ch_array, DOC[topic][i] | defaulttopic, MAX_TOPIC_SIZE);
    dest_array[i + shift] = ch_array;
}
void update_sketch_parameters_flash()
{
    /* Custom paramters for each sketch used IOT2*/
    yield();
}
void update_Parameters_flash()
{
    StaticJsonDocument<1250> DOC;

    /* Part A: update filenames of paramter files */
    iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0]));

    /* Part B: Read from flash, and update myIOT parameters */
    iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC);
    iot.update_vars_flash_parameters(DOC);
    DOC.clear();

    uint8_t accum_shift = 0;

    /* Part C: Read Topics from flash, and update myIOT Topics */
    iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC); /* extract topics from flash */
    updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics", "myHome/Messages", sizeof(topics_gen_pub) / (sizeof(topics_gen_pub[0])));
    updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics", "myHome/log", sizeof(topics_pub) / (sizeof(topics_pub[0])));
    updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics", "myHome/log", sizeof(topics_sub) / (sizeof(topics_sub[0])));

    accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
    updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", "myHome/log", sizeof(winGroupTopics) / (sizeof(winGroupTopics[0])),accum_shift);

    // accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
    // updateTopics_flash(DOC, buttGroupTopics, iot.topics_sub, "sub_topics_SW_g", "myHome/log", sizeof(buttGroupTopics) / (sizeof(buttGroupTopics[0])), accum_shift);

    // accum_shift += sizeof(buttGroupTopics) / (sizeof(buttGroupTopics[0]));
    // for (uint8_t i = 0; i < numW; i++)
    // {
    //     updateTopics_flash(DOC, winSW_V[i]->name, iot.topics_sub, "sub_topics_win", "myHome/log", i, accum_shift + i);
    // }

    // accum_shift += numW;
    // for (uint8_t i = 0; i < numSW; i++)
    // {
    //     updateTopics_flash(DOC, SW_v[i]->Topic, iot.topics_sub, "sub_topics_SW", "myHome/log", i, accum_shift + i);
    // }

    Serial.begin(115200);
    for (uint8_t i = 0; i < 20; i++)
    {
        Serial.print(i);
        Serial.println(iot.topics_sub[i]);
    }

    DOC.clear();

    /* Part D: Read Sketch paramters from flash, and update Sketch */
    iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC);
    update_sketch_parameters_flash();
    DOC.clear();
}

// ±±±±±±± Genereal pub topic ±±±±±±±±±
const char *topicLog = "myHome/log";
const char *topicDebug = "myHome/debug";
const char *topicmsg = "myHome/Messages";

// ±±±±±±±±±±±± sub Topics ±±±±±±±±±±±±±±±±±±
const char *topicClient = "myHome/Cont_A1";
const char *topicAll = "myHome/All";

// ±±±±±±±±±±±±±±±± Client state pub topics ±±±±±±±±±±±±±±±±
const char *topicClient_avail = "myHome/Cont_A1/Avail";
const char *topicClient_state = "myHome/Cont_A1/State";

const char *EntTypes[2] = {"win", "sw"}; /* Prefix to address client types when using MQTT */

void updateTopics_local()
{
    iot.topics_gen_pub[0] = topicmsg;
    iot.topics_gen_pub[1] = topicLog;
    iot.topics_gen_pub[2] = topicDebug;

    iot.topics_pub[0] = topicClient_avail;
    iot.topics_pub[1] = topicClient_state;

    iot.topics_sub[0] = topicClient;
    iot.topics_sub[1] = topicAll;

    /* Entities*/
    uint8_t shift1 = 2;
    uint8_t groptop = 4;
    uint8_t shift2 = shift1 + numW;
    uint8_t shift3 = shift2 + groptop;
    uint8_t butgroup = 3;
    uint8_t shift4 = shift3 + numSW;

    for (uint8_t i = 0; i < numW; i++)
    {
        iot.topics_sub[i + shift1] = winSW_V[i]->name;
    }
    for (uint8_t i = 0; i < groptop; i++)
    {
        iot.topics_sub[i + shift2] = winGroupTopics[i];
    }

    for (uint8_t i = 0; i < numSW; i++)
    {
        iot.topics_sub[i + shift3] = SW_v[i]->Topic;
    }
    for (uint8_t i = 0; i < butgroup; i++)
    {
        iot.topics_sub[i + shift4] = buttGroupTopics[i];
    }
}
void update_Parameters_local()
{
    iot.useOTA = true;
    iot.useSerial = true;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useFlashP = false;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;
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
    // updateTopics_local();
    // update_Parameters_local();
    update_Parameters_flash();
    iot.start_services(addiotnalMQTT);
}