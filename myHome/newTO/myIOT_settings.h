#include <myIOT2.h>
myIOT2 iot;

extern void simplifyClock(char *days, char *clk, char retVal[25]);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];

    if (strcmp(incoming_msg, "status") == 0)
    {
        char msg2[100];
        sprintf(msg, "Status: ");
        for (int i = 0; i < numSW; i++)
        {
            char s1[15], s2[7];
            char clk[60], clk2[25], clk3[25];
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
                    sprintf(state, "Power[%d/%d]", TOsw[i]->pCounter, (int)TOsw[i]->max_pCount);
                }
                else
                {
                    sprintf(state, "[%s]", "OFF");
                }
            }
            if (TOsw[i]->remTime() > 0)
            {
                char clk[25];
                iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                simplifyClock(s2, s1, clk2);
                sprintf(clk, "started[%s] remain[%s] ", iot.get_timeStamp(clk3, TOsw[i]->onClk()), clk2);
            }
            else
            {
                sprintf(clk, "");
            }

            sprintf(msg2, "[%s] %s %s", sw_names[i], state, clk);
            strcat(msg, msg2);
        }
        Serial.println(msg);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "show_flash_param") == 0)
    {
        char temp[400];
        char temp3[450];
        char *a[] = {iot.myIOT_paramfile, sketch_paramfile};
        iot.pub_debug("~~~Start~~~");
        for (int e = 0; e < sizeof(a) / sizeof(a[0]); e++)
        {
            char *fParams = iot.export_fPars(a[e], sketchJSON, JSON_SIZE_SKETCH);
            strcpy(temp, fParams); /* select the bigger file */
            sprintf(temp3, "%s: %s", a[e], temp);
            iot.pub_debug(temp3);
            sketchJSON.clear();
            delete[] fParams;
        }
        iot.pub_debug("~~~End~~~");
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], timeoutSw[%s]", VER, TOsw[0]->Ver);
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
        sprintf(msg, "Help2: Commands #3 - [{i,on}, {i,off}, {i,remain}, all_off, all_on, {i, timeout,minutes,_pwm}]");
        iot.pub_msg(msg);
    }

    else
    {
        if (iot.num_p > 1)
        {
            if (strcmp(iot.inline_param[1], "timeout") == 0)
            {
                if (!outputPWM[atoi(iot.inline_param[0])])
                {
                    TOsw[atoi(iot.inline_param[0])]->start_TO(atoi(iot.inline_param[2]), 2);
                }
                else
                {
                    if (iot.num_p > 3)
                    {
                        TOsw[atoi(iot.inline_param[0])]->pCounter = atoi(iot.inline_param[3]);
                    }
                    else
                    {
                        TOsw[atoi(iot.inline_param[0])]->pCounter = defPWM[atoi(iot.inline_param[0])];
                    }
                    TOsw[atoi(iot.inline_param[0])]->start_TO(atoi(iot.inline_param[2]), 2);
                }
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (!outputPWM[atoi(iot.inline_param[0])])
                {
                    if (iot.num_p == 2)
                    {
                        TOsw[atoi(iot.inline_param[0])]->start_TO(TOsw[atoi(iot.inline_param[0])]->maxON_minutes, 2); /* max time*/
                    }
                    else
                    {
                        TOsw[atoi(iot.inline_param[0])]->start_TO(atoi(iot.inline_param[0]), 2); /* define time in minutes */
                    }
                }
                else /* Define PWM level */
                {
                    if (iot.num_p == 2)
                    {
                        TOsw[atoi(iot.inline_param[0])]->pCounter = 2;
                    }
                    else
                    {
                        TOsw[atoi(iot.inline_param[0])]->pCounter = atoi(iot.inline_param[2]); // Power Level
                    }
                    TOsw[atoi(iot.inline_param[0])]->start_TO(TOsw[atoi(iot.inline_param[0])]->maxON_minutes, 2);
                }
            }
            else if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                char s1[15], s2[7];
                char clk[60], clk2[25], clk3[25];
                int i = atoi(iot.inline_param[0]);
                if (TOsw[i]->remTime() > 0)
                {
                    iot.convert_epoch2clock(TOsw[i]->remTime(), 0, s1, s2);
                    simplifyClock(s2, s1, clk2);
                    // iot.get_timeStamp(clk3, TOsw[i]->onClk());
                    sprintf(clk, "MQTT: remain [%s] ", clk2);
                    iot.pub_msg(clk);
                }
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                TOsw[atoi(iot.inline_param[0])]->finish_TO(2);
            }
            else if (strcmp(iot.inline_param[1], "addTO") == 0)
            {
                TOsw[atoi(iot.inline_param[0])]->add_TO(atoi(iot.inline_param[2]), 2);
            }
        }
    }
}
void startIOTservices()
{
    iot.useFlashP = true;
    iot.start_services(addiotnalMQTT);
}