myIOT2 iot;

extern void _turnOFF_cb(uint8_t i, uint8_t type);
extern void _turnON_cb(uint8_t i, uint8_t type);

// ±±±±±±± Genereal pub topic ±±±±±±±±±
const char *topicLog = "myHome/log";
const char *topicDebug = "myHome/debug";
const char *topicmsg = "myHome/Messages";

// ±±±±±±±±±±±± sub Topics ±±±±±±±±±±±±±±±±±±
const char *topicSub1 = "myHome/alarmMonitor";
const char *topicClient = "myHome/SWPanels/SaloonLights";
const char *topicAll = "myHome/All";

// ±±±±±±±±±±±±±±±± Client state pub topics ±±±±±±±±±±±±±±±±
const char *topicClient_avail = "myHome/SWPanels/SaloonLights/Avail";
const char *topicClient_state = "myHome/SWPanels/SaloonLights/State";

void updateState(uint8_t i, bool state)
{
    char t[60];
    char r[5];
    sprintf(t, "%s%d", topicClient_state, i);
    sprintf(r, "%d", state);
    iot.pub_noTopic(r, t, true);
}
void updateTopics_local()
{
    iot.topics_gen_pub[0] = topicmsg;
    iot.topics_gen_pub[1] = topicLog;
    iot.topics_gen_pub[2] = topicDebug;

    iot.topics_pub[0] = topicClient_avail;
    iot.topics_pub[1] = topicClient_state;

    iot.topics_sub[0] = topicClient;
    iot.topics_sub[1] = topicAll;
    iot.topics_sub[2] = topicSub1;
}
void update_Parameters_local()
{
    iot.useOTA = true;
    iot.useSerial = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useFlashP = false;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;
}

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];

    if (strcmp(incoming_msg, "status") == 0)
    {
        char b[30];
        sprintf(msg, "[Status]:");
        for (uint8_t i = 0; i < numSW; i++)
        {
            sprintf(b, " [%s] is [%s]%s", ButtonNames[i], digitalRead(relayPins[i]) ? "ON" : "OFF", i == numSW - 1 ? "" : ";");
            strcat(msg, b);
        }
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2:<i>,on; <i>,off; all_off");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: %s, useButton[%d], useRFremote[%d]", ver, useButton, useRF);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "all_off") == 0)
    {
        for (uint8_t i = 0; i < numSW; i++)
        {
            _turnOFF_cb(i, MQTT);
        }
    }
    else
    {
        uint8_t i = atoi(iot.inline_param[0]);
        if (iot.num_p > 1 && atoi(iot.inline_param[0]) < numSW)
        {
            if (strcmp(iot.inline_param[1], "off") == 0)
            {
                _turnOFF_cb(i, MQTT);
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                _turnON_cb(i, MQTT);
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
