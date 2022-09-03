myIOT2 iot;

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
        iot.topics_sub[i + shift1] = winTopics[i];
    }
    for (uint8_t i = 0; i < groptop; i++)
    {
        iot.topics_sub[i + shift2] = winGroupTopics[i];
    }

    for (uint8_t i = 0; i < numSW; i++)
    {
        iot.topics_sub[i + shift3] = buttTopics[i];
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

void _gen_WinMSG(uint8_t state, uint8_t reason, uint8_t i)
{
    char msg[100];
    sprintf(msg, "Window [#%d] [%s] is [%s] by [%s]", i, winTopics[i], STATES_TXT[state], REASONS_TXT[reason]);
    iot.pub_msg(msg);
}
void updateState(uint8_t i, bool state) /* Button State MQTT update */
{
    char t[60];
    char r[5];
    sprintf(t, "%s%d", topicClient_state, i);
    sprintf(r, "%d", state);
    iot.pub_noTopic(r, t, true);
}
void _pub_turn(uint8_t i, uint8_t type, bool request)
{
    char msg[50];
    sprintf(msg, "[%s]: [SW#%d][%s] Turn [%s]", turnTypes[type], i, ButtonNames[i], request == HIGH ? "ON" : "OFF");
    iot.pub_msg(msg);
    updateState(i, (int)request);
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
        if (strcmp(iot.inline_param[0], EntTypes[0]) == 0 && (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0 || strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0 || strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0)) /* MQTT cmd for windows */
        {
            uint8_t _word = 0;
            if (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0)
            {
                _word = UP;
            }
            else if (strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0)
            {
                _word = DOWN;
            }
            else if (strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0)
            {
                _word = STOP;
            }
            else
            {
                return;
            }

            for (uint8_t i = 0; i < numW; i++)
            {
                if (strcmp(_topic, winTopics[i]) == 0)
                {
                    winSW_V[i]->ext_SW(_word, MQTT);
                    return;
                }
                else
                {
                    return;
                }
            }
            for (uint8_t i = 0; i < sizeof(winGroupTopics) / sizeof(winGroupTopics[0][0]); i++)
            {
                for (uint8_t i = 0; i < numW; i++)
                {
                    winSW_V[i]->ext_SW(_word, MQTT);
                    return;
                }
            }
        }
        else if (strcmp(iot.inline_param[0], EntTypes[1]) == 0 && (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0 || strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0)) /* MQTT cmd for SW */
        {
            for (uint8_t i = 0; i < numSW; i++)
            {
                if (strcmp(_topic, buttTopics[i]) == 0)
                {
                    if (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0)
                    {
                        _turnON_cb(i, _MQTT);
                        return;
                    }
                    else if (strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0)
                    {
                        _turnOFF_cb(i, _MQTT);
                        return;
                    }
                }
                else
                {
                    return;
                }
            }
            for (uint8_t i = 0; i < sizeof(buttGroupTopics) / sizeof(buttGroupTopics[0][0]); i++)
            {
                for (uint8_t i = 0; i < numSW; i++)
                {
                    if (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0)
                    {
                        _turnON_cb(i, _MQTT);
                        return;
                    }
                    else if (strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0)
                    {
                        _turnOFF_cb(i, _MQTT);
                        return;
                    }
                }
            }
        }
    }
}
void startIOTservices()
{
    updateTopics_local();
    update_Parameters_local();
    iot.start_services(addiotnalMQTT);
}