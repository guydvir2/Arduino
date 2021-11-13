myIOT2 iot;

extern void allOff();
extern uint8_t get_systemState();
extern void set_armState(uint8_t req_state);

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEV_TOPIC "alarmMonitor"
#define PREFIX_TOPIC "myHome"
#define GROUP_TOPIC ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

void giveStatus(char *state)
{
    char t3[50];
    uint8_t cstate = get_systemState();

    sprintf(t3, "");
    // relays state
    if (cstate == ERROR)
    {
        strcat(t3, "Status: invalid [Armed] and [Away] State");
    }
    else if (cstate == ARMED_KEYPAD)
    {
        strcat(t3, "Status: Manual [Armed]");
    }
    else if (cstate == ARMED_HOME_CODE)
    {
        strcat(t3, "Status: [Code] [Home Armed]");
    }
    else if (cstate == ARMED_AWAY_CODE)
    {
        strcat(t3, "Status: [Code] [Armed Away]");
    }
    else if (cstate == ALARMING)
    {
        strcat(t3, "Status: [Alarm]");
    }
    else if (cstate == DISARMED)
    {
        strcat(t3, "Status: [disarmed]");
    }
    else
    {
        strcat(t3, "Status: [notDefined]");
    }
    sprintf(state, "%s", t3);
}
void addiotnalMQTT(char *incoming_msg)
{
    char msg[100];

    if (strcmp(incoming_msg, "status") == 0)
    {
        giveStatus(msg);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, sys_states[0]) == 0)
    {
        set_armState(ARMED_HOME_CODE);
    }
    else if (strcmp(incoming_msg, sys_states[1]) == 0)
    {
        set_armState(ARMED_AWAY_CODE);
    }
    else if (strcmp(incoming_msg, sys_states[2]) == 0)
    {
        set_armState(DISARMED);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver:[%s]", VER);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "pins") == 0)
    {
        sprintf(msg, "Switch: SYS_IS_ARMED_INDICATION_PIN[%d] SYS_IS_ALARMING_INDICATION_PIN[%d], Relay: output_home[%d] output_full[%d]",
                SYS_IS_ARMED_INDICATION_PIN, SYS_IS_ALARMING_INDICATION_PIN, SET_SYSTEM_ARMED_HOME_PIN, SET_SYSTEM_ARMED_AWAY_PIN);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "clear") == 0)
    {
        digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, STATE_ON);
        delay(systemPause);
        digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, !STATE_ON);
        digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, STATE_ON);
        delay(systemPause);
        digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, !STATE_ON);

        iot.sendReset("Reset via MQTT");
    }
    else if (strcmp(incoming_msg, "all_off") == 0)
    {
        allOff();
        sprintf(msg, "All OFF: Received from %s", "MQTT");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #1 - [status, boot, reset, ip, ota, ver2, help, pins]");
        iot.pub_msg(msg);
        sprintf(msg, "Help2: Commands #2 - [armed_home, armed_away, disarmed, clear, all_off, debug]");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "debug") == 0)
    {
        sprintf(msg, "armPin is [%d], AlarmPin is [%d], ArmHome is [%d], ArmAway is [%d]", digitalRead(SYS_IS_ARMED_INDICATION_PIN),
                digitalRead(SYS_IS_ALARMING_INDICATION_PIN), digitalRead(SET_SYSTEM_ARMED_HOME_PIN), digitalRead(SET_SYSTEM_ARMED_AWAY_PIN));
        iot.pub_msg(msg);
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 30;
    iot.useBootClockLog = true;
    iot.useAltermqttServer = false;
    iot.ignore_boot_msg = false;

    iot.deviceTopic = DEV_TOPIC;
    iot.prefixTopic = PREFIX_TOPIC;
    iot.addGroupTopic = GROUP_TOPIC;

    iot.start_services(addiotnalMQTT);
}