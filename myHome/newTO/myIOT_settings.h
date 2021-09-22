extern void simplifyClock(char *days, char *clk, char retVal[25]);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[60];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "Status: ");
        for (int i = 0; i < numSW; i++)
        {
            char s1[15], s2[7];
            char clk[60], clk2[25];
            char state[15];
            if (outputPWM[i] == false)
            {
                bool Q = digitalRead(outputPin[i]);
                sprintf(state, "[%s]", Q == output_ON[i] ? "ON" : "OFF");
            }
            else
            {
                if (TOsw[i]->remTime() > 0)
                {
                    sprintf(clk2, "Power[%d/%d]", TOsw[i]->pCounter, (int)TOsw[i]->max_pCount);
                    strcpy(state, clk2);
                }
                else
                {
                    sprintf(state, "[%s]", "OFF");
                }
            }

            if (TOsw[i]->remTime() > 0)
            {
                iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                simplifyClock(s2, s1, clk2);
                iot.get_timeStamp(TOsw[i]->onClk());
                sprintf(clk, "started[%s] remain[%s] ", iot.timeStamp, clk2);
            }
            else
            {
                sprintf(clk, "");
            }

            sprintf(msg2, "[%s] %s %s", sw_names[i], state, clk);
            strcat(msg, msg2);
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
            strcpy(temp, iot.export_fPars(a[e], sketchJSON, 1000)); /* select the bigger file */
            sprintf(temp3, "%s: %s", a[e], temp);
            iot.pub_debug(temp3);
            sketchJSON.clear();
        }
        iot.pub_debug("~~~End~~~");
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], lib: [%s], timeoutSw[%s]", VER, iot.ver, TOsw[0]->VER);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "all_off") == 0)
    {
        for (int i = 0; i < numSW; i++)
        {
            TOsw[i]->finish_TO(2);
        }
    }
    else if (strcmp(incoming_msg, "all_on") == 0)
    {
        for (int i = 0; i < numSW; i++)
        {
            TOsw[i]->start_TO(TOsw[i]->def_TO_minutes, 2);
        }
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #3 - [{on,i}, {off,i}, all_off, all_on, {timeout,minutes,i}, {remain,i}, {show_flash_param}]");
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (num_parameters > 1)
        {
            if (strcmp(iot.inline_param[1], "timeout") == 0)
            {
                if (outputPWM[atoi(iot.inline_param[0])] == false)
                {
                    TOsw[atoi(iot.inline_param[0])]->start_TO(atoi(iot.inline_param[2]), 2);
                    Serial.println("HERE");
                }
                else
                {
                    TOsw[atoi(iot.inline_param[0])]->pCounter = atoi(iot.inline_param[3]);
                    TOsw[atoi(iot.inline_param[0])]->start_TO(atoi(iot.inline_param[2]), 2);
                }
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (outputPWM[atoi(iot.inline_param[0])] == false)
                {
                    if (num_parameters == 2)
                    {
                        TOsw[atoi(iot.inline_param[0])]->start_TO(TOsw[atoi(iot.inline_param[0])]->maxON_minutes, 2); /* max time*/
                    }
                    else
                    {
                        TOsw[atoi(iot.inline_param[0])]->start_TO(atoi(iot.inline_param[0]), 2); /* define time in minutes */
                    }
                }
                else /* Define power level */
                {
                    TOsw[atoi(iot.inline_param[0])]->pCounter = atoi(iot.inline_param[2]); // Power Level
                    TOsw[atoi(iot.inline_param[0])]->start_TO(TOsw[atoi(iot.inline_param[0])]->maxON_minutes, 2);
                }
            }
            else if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                char s1[15], s2[7];
                char clk[60], clk2[25];
                int i = atoi(iot.inline_param[0]);
                if (TOsw[i]->remTime() > 0)
                {
                    iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                    simplifyClock(s2, s1, clk2);
                    iot.get_timeStamp(TOsw[i]->onClk());
                    sprintf(clk, "remain[%s] ", clk2);
                    iot.pub_msg(clk);
                }
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                TOsw[atoi(iot.inline_param[0])]->finish_TO(2);
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
