myIOT2 iot;
extern homeCtl controller;

/* ±±±±±±±±±±±±±± Receiving and handling MQTT CMDs ±±±±±±±±±±±±±±±± */
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

char topics_sub[3][MAX_TOPIC_SIZE];     /* generic topic array */
char topics_pub[2][MAX_TOPIC_SIZE];     /* generic topic array */
char topics_gen_pub[3][MAX_TOPIC_SIZE]; /* generic topic array */
char winGroupTopics[2][MAX_TOPIC_SIZE]; /* group topic array */
char SwGroupTopics[3][MAX_TOPIC_SIZE];  /* group topic array */

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/* MQTT handlers */
void MQTT_clear_retained(char *topic)
{
    iot.pub_noTopic("", topic, true);
}
void MQTT_update_state(uint8_t state, char *name) /* Windows State MQTT update */
{
    char t[60];
    char r[5];
    sprintf(t, "%s/State", name);
    sprintf(r, "%d", state);
    iot.pub_noTopic(r, t, true);
}
void post_telemetry_2MQTT(Cotroller_Ent_telemetry &MSG) /* get telemetry from any entity */
{
    char msg[100];

    if (MSG.type == WIN_ENT)
    {
        Win_props win_props;

        controller.get_entity_prop(MSG.type /* window */, MSG.id /* win entity id */, win_props);
        if (!win_props.virtCMD)
        {
            sprintf(msg, "[%s]: [WIN#%d] [%s] turned [%s]", controller.WinTrigs[MSG.trig], MSG.id, win_props.name, controller.WinStates[MSG.state]);
            iot.pub_msg(msg);

#if RETAINED_MSG
            iot.pub_noTopic(/*controller.winMQTTcmds[MSG.state]*/ "", win_props.name, true); /* Retain Message */
#endif
            MQTT_update_state(MSG.state, win_props.name); /* Retain State */
        }
        else
        {
            iot.pub_noTopic(controller.winMQTTcmds[MSG.state], win_props.name, true); // <---- Fix this : off cmd doesnot appear
        }
    }
    else if (MSG.type == SW_ENT)
    {
        SW_props sw_props;
        controller.get_entity_prop(MSG.type /* Sw */, MSG.id /* Sw entity id */, sw_props);

        if (!sw_props.virtCMD)
        {
            char msg2[30];

            if (MSG.state == 1 && sw_props.timeout) /* On, with timeout */
            {
                char t[20];
                iot.convert_epoch2clock((int)((MSG.timeout + 500) / 1000), 0, t);
                sprintf(msg2, "timeout [%s]", t);
            }
            else if (MSG.state == 1 && !sw_props.timeout) /* On, without timeout*/
            {
                strcpy(msg2, "timeout [No]");
            }
            else if (MSG.state == 0) /* Off*/
            {
                strcpy(msg2, "");
            }

            sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s] %s",
                    controller.SW_Types[MSG.trig], sw_props.id, sw_props.name, controller.SW_MQTT_cmds[MSG.state], msg2);
            iot.pub_msg(msg);

#if RETAINED_MSG
            iot.pub_noTopic("" /*controller.SW_MQTT_cmds[MSG.state]*/, sw_props.name, true); /* Retain Message */
#endif
            MQTT_update_state(MSG.state, sw_props.name); /* Retain State */
        }
        else
        {
            iot.pub_noTopic(controller.SW_MQTT_cmds[MSG.state], sw_props.name, true); /* Retained */
            sprintf(msg, "[%s]: Switched [%s] Virtual [%s]", controller.SW_Types[sw_props.type], controller.SW_MQTT_cmds[MSG.state], sw_props.name);
            iot.pub_msg(msg);
        }
    }
}

/* CHECK TOPICS - for what entity it belongs */
bool MQTT_is_Contl_topic(char *_topic)
{
    if (strcmp(_topic, topics_sub[0]) == 0 /* Cont Topic */ || strcmp(_topic, topics_sub[1]) == 0 /* All Topic */)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
bool MQTT_is_lockdown_topic(char *_topic)
{
    if (strcmp(_topic, topics_sub[2]) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
bool MQTT_is_Win_topic(char *_topic, uint8_t &n)
{
    for (uint8_t i = 0; i < controller.get_ent_counter(WIN_ENT); i++)
    {
        Win_props win_props;
        controller.get_entity_prop(WIN_ENT /* window */, i /* win entity id */, win_props);
        if (strcmp(_topic, win_props.name) == 0) /* SENT FOR A SPECIFIC TOPIC */
        {
            n = i;
            return 1;
        }
    }
    return 0;
}
bool MQTT_is_SW_topic(char *_topic, uint8_t &n)
{
    for (uint8_t i = 0; i < controller.get_ent_counter(SW_ENT); i++)
    {
        SW_props sw_props;
        controller.get_entity_prop(SW_ENT /* SW */, i /* win entity id */, sw_props);

        if (strcmp(_topic, sw_props.name) == 0) /* SENT FOR A SPECIFIC TOPIC */
        {
            n = i;
            return 1;
        }
    }
    return 0;
}
bool MQTT_is_Wingroup_topic(char *_topic)
{
    for (uint8_t i = 0; i < sizeof(winGroupTopics) / sizeof(winGroupTopics[0]); i++) /* OR SENT FOR A GROUP */
    {
        if (strcmp(_topic, winGroupTopics[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}
bool MQTT_is_SWgroup_topic(char *_topic)
{
    for (uint8_t i = 0; i < sizeof(SwGroupTopics) / sizeof(SwGroupTopics[0]); i++) /* SENT FOR A GROUP TOPIC */
    {
        if (strcmp(_topic, SwGroupTopics[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* Answer Status CMD */
void MQTT_Post_Win_status(uint8_t i, char *msg)
{
    Win_props win_props;
    Cotroller_Ent_telemetry telemetry;
    controller.get_entity_prop(WIN_ENT, i, win_props);
    controller.get_telemetry(telemetry);

    sprintf(msg, "Status: Win[#%d][%s] [%s]%s", i, win_props.name, controller.winMQTTcmds[controller.get_ent_state(WIN_ENT, i)], i == (controller.get_ent_counter(WIN_ENT) - 1) ? "" : "; ");
    iot.pub_msg(msg);
}
void MQTT_WinGroup_status(char *msg)
{
    for (uint8_t i = 0; i < controller.get_ent_counter(WIN_ENT); i++)
    {
        MQTT_Post_Win_status(i, msg);
    }
}
void MQTT_Post_SW_status(uint8_t i, char *msg)
{
    SW_props sw_props;
    Cotroller_Ent_telemetry telemetry;
    controller.get_entity_prop(SW_ENT, i, sw_props);
    controller.get_telemetry(telemetry);

    sprintf(msg, "Status: SW[#%d][%s] [%s]", i, sw_props.name, controller.SW_MQTT_cmds[controller.get_ent_state(SW_ENT, i)]); //, i == swEntityCounter - 1 ? "" : "; ");
    iot.pub_msg(msg);
}
void MQTT_SWGroup_status(char *msg)
{
    for (uint8_t i = 0; i < controller.get_ent_counter(SW_ENT); i++)
    {
        MQTT_Post_SW_status(i, msg);
    }
}

/* Switch Windows CMDs*/
bool MQTT_is_valid_winState(char *inmsg, uint8_t &ret_state)
{
    if (strcmp(inmsg, controller.winMQTTcmds[1]) == 0) /* UP */
    {
        ret_state = UP;
        return 1;
    }
    else if (strcmp(inmsg, controller.winMQTTcmds[2]) == 0) /* DOwN */
    {
        ret_state = DOWN;
        return 1;
    }
    else if (strcmp(inmsg, controller.winMQTTcmds[0]) == 0) /* STOP */
    {
        ret_state = STOP;
        return 1;
    }
    else
    {
        return 0;
    }
}
void MQTT_WinGroup_Change_state(uint8_t state)
{
    for (uint8_t i = 0; i < controller.get_ent_counter(WIN_ENT); i++)
    {
        controller.Win_switchCB(i, state);
    }
}

/* Switch SW CMDs*/
bool MQTT_is_valid_SWstate(char *inmsg, uint8_t &ret_state)
{
    if (strcmp(inmsg, controller.SW_MQTT_cmds[1]) == 0) /* ON */
    {
        ret_state = 1;
        return 1;
    }
    else if (strcmp(inmsg, controller.SW_MQTT_cmds[0]) == 0) /* OFF */
    {
        ret_state = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
void MQTT_SWGroup_Change_state(uint8_t state)
{
    for (uint8_t i = 0; i < controller.get_ent_counter(SW_ENT); i++)
    {
        controller.SW_switchCB(i, state);
    }
}

/* Entity description */
void MQTT_Win_entity(uint8_t i, char *msg)
{
    Win_props win_props;
    controller.get_entity_prop(WIN_ENT, i, win_props);
    sprintf(msg, "[Entity]: [Win#%d] topic [%s], Virtual-out [%s], 2nd-input [%s]",
            win_props.id, win_props.name, win_props.virtCMD ? "Yes" : "No", win_props.extSW ? "Yes" : "No");
    iot.pub_msg(msg);
}
void MQTT_SW_entity(uint8_t i, char *msg)
{
    SW_props sw_props;
    controller.get_entity_prop(SW_ENT, i, sw_props);

    char *w[] = {"None", "On_off SW", "Push Button"};
    sprintf(msg, "[Entity]: [SW#%d] topic [%s], Virtual-out [%s], type [%s], timeout [%s]",
            sw_props.id, sw_props.name, sw_props.virtCMD ? "Yes" : "No", w[sw_props.type], sw_props.timeout ? "Yes" : "No");
    iot.pub_msg(msg);
}

/* Controller CMDS */
void MQTT_to_controller(char *incoming_msg, char *msg)
{
    if (strcmp(incoming_msg, "status") == 0)
    {
        MQTT_WinGroup_status(msg);
        MQTT_SWGroup_status(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2: {Controller : [status, ver2, help2, entities]}, {Windows : [up, down, off]}, {Switch : [on, off]}");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2:[%s], [%s], [%s]", controller.ver, controller.get_ent_ver(0), controller.get_ent_ver(1));
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "entities") == 0)
    {
        for (uint8_t n = 0; n < controller.get_ent_counter(WIN_ENT); n++)
        {
            MQTT_Win_entity(n, msg);
        }
        for (uint8_t n = 0; n < controller.get_ent_counter(SW_ENT); n++)
        {
            MQTT_SW_entity(n, msg);
        }
    }
}
void MQTT_to_lockdown(char *incoming_msg)
{
    if (strcmp(incoming_msg, "true") == 0)
    {
        controller.Win_init_lockdown();
    }
    if (strcmp(incoming_msg, "false") == 0)
    {
        controller.Win_release_lockdown();
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    uint8_t n = 255;
    uint8_t state;
    char msg[150];
    if (MQTT_is_Contl_topic(_topic))
    {
        MQTT_to_controller(incoming_msg, msg);
    }
    else if (MQTT_is_lockdown_topic(_topic))
    {
        MQTT_to_lockdown(incoming_msg);
    }
    else if (MQTT_is_Win_topic(_topic, n))
    {
        if (MQTT_is_valid_winState(incoming_msg, state))
        {
            controller.Win_switchCB(n, state);
        }
        else if (strcmp(incoming_msg, "status") == 0)
        {
            MQTT_Post_Win_status(n, msg);
        }
    }
    else if (MQTT_is_SW_topic(_topic, n))
    {
        if (MQTT_is_valid_SWstate(incoming_msg, state))
        {
            controller.SW_switchCB(n, state);
        }
        else if (strcmp(incoming_msg, "status") == 0)
        {
            MQTT_Post_SW_status(n, msg);
        }
        else
        {
            if (iot.num_p > 1 && strcmp(iot.inline_param[0], "timeout") == 0)
            {
                controller.SW_switchCB(n, 1, atoi(iot.inline_param[1]));
            }
        }
    }
    else if (MQTT_is_Wingroup_topic(_topic))
    {
        if (MQTT_is_valid_winState(incoming_msg, state))
        {
            MQTT_WinGroup_Change_state(state);
        }
        else if (strcmp(incoming_msg, "status") == 0)
        {
            MQTT_WinGroup_status(msg);
        }
    }
    else if (MQTT_is_SWgroup_topic(_topic))
    {
        if (MQTT_is_valid_SWstate(incoming_msg, state))
        {
            MQTT_SWGroup_Change_state(state);
        }
        else if (strcmp(incoming_msg, "status") == 0)
        {
            MQTT_SWGroup_status(msg);
        }
        else
        {
            /* Here Timeout CMD */
        }
    }
}
void startIOTservices()
{
    iot.start_services(addiotnalMQTT);
}