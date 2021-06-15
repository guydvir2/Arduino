myIOT2 iot;
extern const int systemPause, deBounceInt;
extern void allOff();
extern void arm_home();
extern void arm_away();
extern void disarmed();

extern byte relays[], inputs[];

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEV_TOPIC "alarmMonitor"
#define PREFIX_TOPIC "myHome"
#define GROUP_TOPIC ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

void giveStatus(char *state)
{
    char t3[50];

    sprintf(t3, "");
    // relays state
    if (digitalRead(OUTPUT1) == RelayOn && digitalRead(OUTPUT2) == RelayOn)
    {
        strcat(t3, "Status: invalid [Armed] and [Away] State");
    }
    else if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn && digitalRead(INPUT1) == SwitchOn)
    {
        strcat(t3, "Status: Manual [Armed]");
    }
    else if (digitalRead(OUTPUT1) == RelayOn && digitalRead(OUTPUT2) == !RelayOn && digitalRead(INPUT1) == SwitchOn)
    {
        strcat(t3, "Status: [Code] [Home Armed]");
    }
    else if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == RelayOn && digitalRead(INPUT1) == SwitchOn)
    {
        strcat(t3, "Status: [Code] [Armed Away]");
    }
    else if (digitalRead(INPUT1) == SwitchOn && digitalRead(INPUT2) == SwitchOn)
    {
        strcat(t3, "Status: [Alarm]");
    }
    else if (digitalRead(INPUT1) == !SwitchOn && digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn)
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
    else if (strcmp(incoming_msg, "armed_home") == 0)
    {
        arm_home();
    }
    else if (strcmp(incoming_msg, "armed_away") == 0)
    {
        arm_away();
    }
    else if (strcmp(incoming_msg, "disarmed") == 0)
    {
        disarmed();
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver:[%s]", VER);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "pins") == 0)
    {
        sprintf(msg, "Switch: input1[%d] input2[%d], Relay: output_home[%d] output_full[%d]", INPUT1, INPUT2, OUTPUT1, OUTPUT2);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "reset") == 0)
    {
        for (int i = 0; i < 2; i++)
        {
            digitalWrite(relays[i], RelayOn);
            delay(systemPause);
            digitalWrite(relays[i], !RelayOn);
        }
        iot.sendReset("Reset via MQTT");
    }
    else if (strcmp(incoming_msg, "all_off") == 0)
    {
        allOff();
        sprintf(msg, "All OFF: Received from %s", "MQTT");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help") == 0)
    {
        sprintf(msg, "Help2: Commands #1 - [status, boot, reset, ip, ota, ver2, help, pins]");
        iot.pub_msg(msg);
        sprintf(msg, "Help2: Commands #2 - [armed_home, armed_away, disarmed, reset, all_off, debug]");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "debug") == 0)
    {
        sprintf(msg, "INPUT1 is [%d], INPUT2 is [%d], OUTPUT1 is [%d], OUTPUT2 is [%d]", digitalRead(INPUT1), digitalRead(INPUT2), digitalRead(OUTPUT1), digitalRead(OUTPUT2));
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
    iot.resetFailNTP = true;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 30;
    
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);
    iot.start_services(addiotnalMQTT);
}