myIOT2 iot;
extern homeCtl controller;

void post_telemetry_2MQTT(Ctl_MSGstr &MSG)
{
    char msg[100];
    char T[MAX_TOPIC_SIZE];
    if (MSG.type == 0)
    {
        if (!MSG.virtCMD)
        {
            controller.get_Win_name(MSG.id, *T);
            sprintf(msg, "[%s]: [WIN#%d] [%s] turned [%s]", REASONS_TXT[MSG.reason], MSG.id, T, STATES_TXT[MSG.state]);
            iot.pub_msg(msg);
        }
    }
    else if (MSG.type == 1)
    {
        if (!MSG.virtCMD)
        {
            char msg[100];
            char msg2[30];

            if (MSG.state == 1 && controller.SW_use_timeout(MSG.id))
            {
                char t[20];
                // iot.convert_epoch2clock((int)((SW_v[i]->get_remain_time() + 500) / 1000), 0, t);
                sprintf(msg2, "timeout [%s]", t);
            }
            else if (!controller.SW_use_timeout(MSG.id))
            {
                strcpy(msg2, "timeout [No]");
            }
            else if (MSG.state  == 0)
            {
                strcpy(msg2, "");
            }

            // sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s] %s", SW_Types[type], i, SW_v[i]->name, request == HIGH ? "ON" : "OFF", msg2);
            // iot.pub_msg(msg);
        }
    }
    // #if RETAINED_MSG
    //     iot.pub_noTopic((char *)winMQTTcmds[state], winSW_V[i]->name, true); /* Retain Message */
    // #endif
    //     Winupdate_MQTT_state(i, state); /* Retain State */
}
void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    uint8_t n;
    uint8_t state;
    char msg[150];
}
void startIOTservices()
{
    iot.start_services(addiotnalMQTT);
}