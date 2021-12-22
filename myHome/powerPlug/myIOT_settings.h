myIOT2 iot;

extern void startTO(uint8_t src, uint8_t i = 0);
extern void endTO(uint8_t src, uint8_t i = 0);
extern void simplifyClock(char *days, char *clk, char retVal[25]);
extern timeOUTSwitch TOswitch;

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    
    if (strcmp(incoming_msg, "status") == 0)
    {
        char state[15];
        char s1[15], s2[7];
        char clk[60], clk2[25];

        sprintf(msg, "Status: ");
        sprintf(state, "[%s]", digitalRead(relayPin) ? "ON" : "OFF");

        if (TOswitch.remTime() > 0)
        {
            iot.convert_epoch2clock(TOswitch.remTime(), 0, s1, s2);
            simplifyClock(s2, s1, clk2);
            iot.get_timeStamp(TOswitch.onClk());
            sprintf(clk, "started[%s] remain[%s] ", iot.get_timeStamp(TOswitch.onClk()), clk2);
        }
        else
        {
            sprintf(clk, "");
        }

        sprintf(msg2, "%s %s", state, clk);
        strcat(msg, msg2);

        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "on") == 0)
    {
        TOswitch.start_TO(TOswitch.def_TO_minutes, 2);
    }
    else if (strcmp(incoming_msg, "off") == 0)
    {
        TOswitch.finish_TO(2);
    }
    else if (strcmp(incoming_msg, "show_flash_param") == 0)
    {
        char temp[300];
        char temp3[350];
        char *a[] = {iot.myIOT_paramfile, sketch_paramfile};
        iot.pub_debug("~~~Start~~~");
        for (int e = 0; e < sizeof(a) / sizeof(a[0]); e++)
        {
            strcpy(temp, iot.export_fPars(a[e], sketchJSON, 1000)); /* select the bigger file */
            sprintf(temp3, "%s: %s", a[e], temp);
            iot.pub_debug(temp3);
            sketchJSON.clear();
        }
        iot.pub_debug("~~~End~~~");
        iot.pub_msg("onFlash Parameters exttracted to debug topic");
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], lib: [%s], timeoutSw[%s]", vER, iot.ver, TOswitch.Ver);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #3 - [on, off, {timeout,minutes}, show_flash_param]");
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (num_parameters > 1)
        {
            if (strcmp(iot.inline_param[0], "timeout") == 0)
            {
                TOswitch.start_TO(atoi(iot.inline_param[1]), 2);
            }
        }
    }
}
void startIOTservices()
{
    iot.useSerial = paramJSON["useSerial"];
    iot.useWDT = paramJSON["useWDT"];
    iot.useOTA = paramJSON["useOTA"];
    iot.useResetKeeper = paramJSON["useResetKeeper"];
    iot.useDebug = paramJSON["useDebugLog"];
    iot.debug_level = paramJSON["debug_level"];
    iot.useNetworkReset = paramJSON["useNetworkReset"];
    iot.noNetwork_reset = paramJSON["noNetwork_reset"];
    iot.useextTopic = paramJSON["useextTopic"];
    iot.useBootClockLog = paramJSON["useBootClockLog"];
    strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
    strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
    strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);

    iot.start_services(addiotnalMQTT);
}
