myIOT2 iot;

// ±±±±±±± Genereal pub topic ±±±±±±±±±
const char *topicLog = "myHome/log";
const char *topicDebug = "myHome/debug";
const char *topicmsg = "myHome/Messages";

// ±±±±±±±±±±±± sub Topics ±±±±±±±±±±±±±±±±±±
const char *topicClient = "myHome/Cont_A1"; // Using controller definition //
const char *topicAll = "myHome/All";

// ±±±±±±±±±±±±±±±± Client state pub topics ±±±±±±±±±±±±±±±±
const char *topicClient_avail = "myHome/Cont_A1/Avail";
const char *topicClient_state = "myHome/Cont_A1/State";

// ±±±±±±±±±±±±±±±± Additioan sub topics ±±±±±±±±±±±±±±±±
// const char *winNick[] = {"Closet", "Bath", "Bed", "TBD"};
const char *topicSubgroup0 = "myHome/Windows";
const char *topicSubgroup1 = "myHome/Windows/gFloor"; // Ground Floor group //
const char *topicSubgroup2 = "myHome/lockdown";
const char *addTopic1 = "myHome/Windows/gFloor/Closet"; // Using controller definition //
const char *addTopic2 = "myHome/Windows/gFloor/Bath";   // Using controller definition //
const char *addTopic3 = "myHome/Windows/gFloor/Bed";    // Using controller definition //
const char *addTopic4 = "myHome/Windows/gFloor/TBD";    // Using controller definition //

// const char *addedTopis[4] = {&addTopic1, &addTopic2, &addTopic3, &addTopic4};
char winGroupTopics[3][30];
char entitiesTopics[maxW][40];

void updateTopics_local()
{
    iot.topics_gen_pub[0] = topicmsg;
    iot.topics_gen_pub[1] = topicLog;
    iot.topics_gen_pub[2] = topicDebug;

    iot.topics_pub[0] = topicClient_avail;
    iot.topics_pub[1] = topicClient_state;

    iot.topics_sub[0] = topicClient;
    iot.topics_sub[1] = topicAll;

    /* Groups */
    strcpy(winGroupTopics[0], topicSubgroup0);
    strcpy(winGroupTopics[1], topicSubgroup1);
    strcpy(winGroupTopics[2], topicSubgroup2);

    iot.topics_sub[2] = winGroupTopics[0];
    iot.topics_sub[3] = winGroupTopics[1];
    iot.topics_sub[4] = winGroupTopics[2];

    /* Entites topics */
    strcpy(entitiesTopics[0], addTopic1);
    strcpy(entitiesTopics[1], addTopic2);
    strcpy(entitiesTopics[2], addTopic3);
    strcpy(entitiesTopics[3], addTopic4);

    /* Entities*/
    for (uint8_t i = 0; i < numW; i++)
    {
        iot.topics_sub[i + 5] = entitiesTopics[i];
    }
    // iot.topics_sub[2] = topicSub1; /* All Windows */

    // iot.topics_sub[3] = addTopic1;
    // iot.topics_sub[4] = addTopic2;
    // iot.topics_sub[5] = addTopic3;
    // iot.topics_sub[6] = addTopic4;
    // iot.topics_sub[7] = addTopic0; /* gFloor (GroundFloor) group*/
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
    else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0)
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
    }
}
void startIOTservices()
{
    updateTopics_local();
    update_Parameters_local();
    iot.start_services(addiotnalMQTT);
}
