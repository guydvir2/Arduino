myIOT2 iot;

extern void allOff();
extern uint8_t get_systemState();
extern void set_armState(uint8_t req_state);

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEV_TOPIC "alarmMonitor"
#define PREFIX_TOPIC "myHome"
#define GROUP_TOPIC ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

void addiotnalMQTT(char *incoming_msg)
{
    char msg[100];

    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "Status: [%s]", sys_states[get_systemState()]);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, sys_states[ARMED_HOME_CODE]) == 0)
    {
        set_armState(ARMED_HOME_CODE);
    }
    else if (strcmp(incoming_msg, sys_states[ARMED_AWAY_CODE]) == 0)
    {
        set_armState(ARMED_AWAY_CODE);
    }
    else if (strcmp(incoming_msg, sys_states[DISARMED]) == 0)
    {
        set_armState(DISARMED);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver:[%s]", VER);
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
        sprintf(msg, "Help2: Commands #1 - [status, ver2, help2]");
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
    iot.useSerial = false;
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

    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

    iot.start_services(addiotnalMQTT);
}