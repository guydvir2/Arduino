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

void _gen_WinMSG(uint8_t state, uint8_t reason, uint8_t i, const char *name = nullptr)
{
    char msg[30];
    sprintf(msg, "Window [#%d] %s is [%s] by [%s]", i, name, STATES_TXT[state], REASONS_TXT[reason]);
    Serial.println(msg);
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
        /* MQTT format MSG:
        win,0,up
        sw,1,off
        */
        uint8_t n = atoi(iot.inline_param[1]);
        if (iot.inline_param[0] == EntTypes[0]) /* MQTT cmd for windows */
        {
            for()
            if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0)
            {
                uint8_t _word = 0;
                if (strcmp(incoming_msg, "up") == 0)
                {
                    _word = UP;
                }
                else if (strcmp(incoming_msg, "down") == 0)
                {
                    _word = DOWN;
                }
                else if (strcmp(incoming_msg, "off") == 0)
                {
                    _word = STOP;
                }

                // if (_topic == addTopic0)
                // {
                //     for (uint8_t i = 0; i < numW; i++)
                //     {
                //         winSW_V[i]->ext_SW(_word, MQTT);
                //         _gen_WinMSG(_word, MQTT, i, _topic);
                //         iot.pub_msg(msg);
                //     }
                // }
                // else if (_topic == addTopic1 || _topic == addTopic2 || _topic == addTopic3 || _topic == addTopic4)
                // {
                //     winSW_V[1]->ext_SW(_word, MQTT);
                //     _gen_WinMSG(_word, MQTT, 1, _topic);
                //     iot.pub_msg(msg);
                // }
                // else
                // {
                //     return;
                // }
                for (uint8_t i = 0; i < numW; i++)
                {
                    if (winSW_V[i]->get_id() == n)
                    {
                        winSW_V[i]->

                    }
                }
            }
        }
        else if (atoi(iot.inline_param[0]) == EntTypes[1]) /* MQTT cmd for SW */
        {
        }
    }
}
void startIOTservices()
{
    updateTopics_local();
    update_Parameters_local();
    iot.start_services(addiotnalMQTT);
}
