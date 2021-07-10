extern void simplifyClock(char *days, char *clk, char retVal[25]);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        bool Q = digitalRead(outputPin[0]);
        char s1[15];
        char s2[7];
        char clk[25];
        if (TOsw[0]->remTime() > 0)
        {
            iot.convert_epoch2clock(TOsw[0]->remTime(), 0, s1, s2);
            simplifyClock(s2, s1, clk);
        }
        else
        {
            sprintf(clk, "None");
        }
        if (TOsw[0]->trigType != 3)
        {
            sprintf(msg, "Status: [%s] remain[%s]", Q == output_ON[0] ? "ON" : "OFF", clk);
        }
        else
        {
            sprintf(msg, "Status: [%s] Power[%d%%] remain[%s]", TOsw[0]->pwm_pCount > 0 ? "ON" : "OFF",
                    (int)(TOsw[0]->pwm_pCount * 100 / TOsw[0]->totPWMsteps), clk);
        }
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "show_flash_param") == 0)
    {
        char temp[300];
        char temp3[350];
        char *a[] = {iot.myIOT_paramfile, sketch_paramfile};
        iot.pub_debug("~~~Start~~~");
        for (int e = 0; e < sizeof(a) / sizeof(a[0]); e++)
        {
            strcpy(temp, iot.export_fPars(a[e], paramJSON));
            sprintf(temp3, "%s: %s", a[e], temp);
            iot.pub_debug(temp3);
            paramJSON.clear();
        }
        iot.pub_debug("~~~End~~~");
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
        // iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [on, off,{timeout,_sec}],show_flash_param");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "remain") == 0)
    {
        sprintf(msg, "Remain: [%d]", TOsw[0]->remTime());
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (num_parameters > 1)
        {
            if (strcmp(iot.inline_param[0], "timeout") == 0)
            {
                if (TOsw[atoi(iot.inline_param[2])]->trigType != 3)
                {
                    TOsw[atoi(iot.inline_param[2])]->start_TO(atoi(iot.inline_param[1]), "MQTT");
                }
                else
                {
                    TOsw[atoi(iot.inline_param[3])]->pwm_pCount = atoi(iot.inline_param[2]);
                    TOsw[atoi(iot.inline_param[3])]->start_TO(atoi(iot.inline_param[1]), "MQTT");
                }
            }
            else if (strcmp(iot.inline_param[0], "on") == 0)
            {
                if (TOsw[atoi(iot.inline_param[1])]->trigType != 3)
                {
                    TOsw[atoi(iot.inline_param[1])]->start_TO(TOsw[atoi(iot.inline_param[1])]->maxON_minutes, "MQTT");
                }
                else
                {
                    TOsw[atoi(iot.inline_param[2])]->pwm_pCount = atoi(iot.inline_param[1]);
                    TOsw[atoi(iot.inline_param[2])]->start_TO(TOsw[atoi(iot.inline_param[1])]->maxON_minutes, "MQTT");
                }
            }
            else if (strcmp(iot.inline_param[0], "off") == 0)
            {
                TOsw[atoi(iot.inline_param[1])]->finish_TO("MQTT");

                // if (TOsw[atoi(iot.inline_param[1])]->trigType != 3)
                // {
                //     TOsw[atoi(iot.inline_param[1])]->finish_TO(TOsw[atoi(iot.inline_param[1])]->maxON_minutes, "MQTT");
                // }
                // else
                // {
                //     TOsw[atoi(iot.inline_param[2])]->pwm_pCount = atoi(iot.inline_param[1]);
                //     TOsw[atoi(iot.inline_param[2])]->finish_TO(TOsw[atoi(iot.inline_param[1])]->maxON_minutes, "MQTT");
                // }
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
    iot.resetFailNTP = paramJSON["useFailNTP"];
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
