myIOT2 iot;
#include <Arduino.h>

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
        delay(DELAY_TO_REACT);
        digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, !STATE_ON);
        delay(DELAY_TO_REACT);

        digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, STATE_ON);
        delay(DELAY_TO_REACT);
        digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, !STATE_ON);
        delay(DELAY_TO_REACT);

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
        sprintf(msg, "armPin is [%s], AlarmPin is [%s], ArmHome is [%s], ArmAway is [%s]",
                digitalRead(SYSTEM_STATE_ARM_PIN) ? "OFF" : "ON",
                digitalRead(SYSTEM_STATE_ALARM_PIN) ? "OFF" : "ON",
                digitalRead(SET_SYSTEM_ARMED_HOME_PIN) ? "ON" : "OFF",
                digitalRead(SET_SYSTEM_ARMED_AWAY_PIN) ? "ON" : "OFF");
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
    iot.noNetwork_reset = 10;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;

    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

    iot.start_services(addiotnalMQTT);
}