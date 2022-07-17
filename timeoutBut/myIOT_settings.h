extern void Ext_trigger_OFF(uint8_t reason, uint8_t i);
extern void Ext_trigger_ON(uint8_t reason, int TO = 0, uint8_t step = 0, uint8_t i = 0);
extern void Ext_updatePWM_value(uint8_t reason, uint8_t step, uint8_t i = 0);
extern void Ext_addTime(uint8_t reason, int timeAdd, uint8_t i = 0);

const char *INPUTS_ORIGIN[4] = {"Timeout", "Button", "MQTT", "PowerON"};

void notifyAdd(int &_add, uint8_t &reason, uint8_t i)
{
    char a[100];
    char clk1[25];
    char clk2[25];
    char clk3[25];
    iot.convert_epoch2clock((_add)*60, 0, clk1);
    iot.convert_epoch2clock((timeoutButtonV[i]->timeout + _add) * 60, 0, clk2);
    iot.convert_epoch2clock(timeoutButtonV[i]->remainWatch() + _add, 0, clk3);
    sprintf(a, "%s: [%s] Added [%s] ,total [%s] remain [%s]", INPUTS_ORIGIN[reason], sw_names[i], clk1, clk2, clk3);
    iot.pub_msg(a);
}
void notifyOFF(uint8_t &reason, uint8_t i)
{
    char a[100];
    char clk[25];
    unsigned int remtime = timeoutButtonV[i]->remainWatch();
    iot.convert_epoch2clock(timeoutButtonV[i]->timeout * 60 - remtime, 0, clk);
    sprintf(a, "%s: [%s] Switched [Off] after [%s]", INPUTS_ORIGIN[reason], sw_names[i], clk);
    if (remtime > 1)
    {
        char b[50];
        iot.convert_epoch2clock(remtime, 0, clk);
        sprintf(b, " Remain [%s]", clk);
        strcat(a, b);
    }
    iot.pub_msg(a);
}
void notifyON(uint8_t &reason, uint8_t i)
{
    char a[100];
    char b[50];
    char clk[25];
    iot.convert_epoch2clock(timeoutButtonV[i]->timeout * 60, 0, clk);
    sprintf(a, "%s: [%s] Switched [On] for [%s]", INPUTS_ORIGIN[reason], sw_names[i], clk);

    if (lightOutputV[i]->isPWM())
    {
        sprintf(b, " Power [%d/%d]", lightOutputV[i]->currentStep, lightOutputV[i]->maxSteps);
        strcat(a, b);
    }

    iot.pub_msg(a);
}
void notifyUpdatePWM(uint8_t &step, uint8_t &reason, uint8_t i)
{
    char b[50];
    if (lightOutputV[i]->isPWM() && timeoutButtonV[i]->getState())
    {
        sprintf(b, "%s: [%s] Power update [%d/%d]", INPUTS_ORIGIN[reason], sw_names[i], step, lightOutputV[i]->maxSteps);
    }
    iot.pub_msg(b);
}
void notifyRemain(uint8_t i)
{
    char msg[50];
    char s1[20];

    int _rem = timeoutButtonV[i]->remainWatch();
    iot.convert_epoch2clock(_rem, 0, s1);
    sprintf(msg, "MQTT: [%s] remain [%s] ", sw_names[i], timeoutButtonV[i]->getState() ? s1 : "Off");
    iot.pub_msg(msg);
}
void status_mqtt(uint8_t i)
{
    char a[150];
    char b[80];
    char pwmstep[25];
    unsigned int rem = timeoutButtonV[i]->remainWatch();

    sprintf(a, "Status: [%s] [%s]", sw_names[i], timeoutButtonV[i]->getState() ? "ON" : "OFF");

    if (lightOutputV[i]->isPWM() && rem > 0)
    {
        sprintf(pwmstep, " Power [%d/%d]", lightOutputV[i]->currentStep, lightOutputV[i]->maxSteps);
        strcat(a, pwmstep);
    }

    if (rem > 0)
    {
        char clk[25];
        char clk2[25];
        iot.convert_epoch2clock(rem, 0, clk);
        iot.convert_epoch2clock(timeoutButtonV[i]->timeout * 60 - rem, 0, clk2);
        sprintf(b, " on-Time [%s], Remain[%s]", clk, clk2);
        strcat(a, b);
    }
    iot.pub_msg(a);
}

#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[3][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];
char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"}; // <----- Verfy file names

void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t ar_size)
{
    for (uint8_t i = 0; i < ar_size; i++)
    {
        strlcpy(ch_array[i], DOC[topic][i] | defaulttopic, MAX_TOPIC_SIZE);
        dest_array[i] = ch_array[i];
    }
}
void update_sketch_parameters_flash(JsonDocument &DOC)
{
    /* Custom paramters for each sketch used IOT2*/

    numSW = DOC["numSW"] | numSW;
    sketch_JSON_Psize = DOC["sketch_JSON_Psize"] | sketch_JSON_Psize;
    PWM_res = DOC["PWM_res"] | PWM_res;

    for (uint8_t i = 0; i < numSW; i++)
    {
        OnatBoot[i] = DOC["OnatBoot"][i] | OnatBoot[i];
        useInput[i] = DOC["useInput"][i] | useInput[i];
        outputPWM[i] = DOC["outputPWM"][i] | outputPWM[i];
        useIndicLED[i] = DOC["useIndicLED"][i] | useIndicLED[i];
        dimmablePWM[i] = DOC["dimmablePWM"][i] | dimmablePWM[i];

        output_ON[i] = DOC["output_ON"][i] | output_ON[i];
        inputPressed[i] = DOC["inputPressed"][i] | inputPressed[i];

        trigType[i] = DOC["trigType"][i] | trigType[i];
        inputPin[i] = DOC["inputPin"][i] | inputPin[i];
        outputPin[i] = DOC["outputPin"][i] | outputPin[i];
        indicPin[i] = DOC["indicPin"][i] | indicPin[i];

        def_TO_minutes[i] = DOC["def_TO_minutes"][i] | def_TO_minutes[i];
        maxON_minutes[i] = DOC["maxON_minutes"][i] | maxON_minutes[i];

        defPWM[i] = DOC["defPWM"][i] | defPWM[i];
        max_pCount[i] = DOC["max_pCount"][i] | max_pCount[i];
        limitPWM[i] = DOC["limitPWM"][i] | limitPWM[i];

        sprintf(sw_names[i], DOC["sw_names"][i] | "LED_err");
    }
}
void update_Parameters_flash()
{
    StaticJsonDocument<1250> DOC;

    iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0])); /* update filenames of paramter files */

    iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC) ? P_readOK_a = true : P_readOK_a = false;
    iot.update_vars_flash_parameters(DOC);
    DOC.clear();

    iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC) ? P_readOK_b = true : P_readOK_b = false; /* extract topics from flash */
    updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics", "myHome/Messages", sizeof(topics_gen_pub) / (sizeof(topics_gen_pub[0])));
    updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics", "myHome/log", sizeof(topics_pub) / (sizeof(topics_pub[0])));
    updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics", "myHome/log", sizeof(topics_sub) / (sizeof(topics_sub[0])));
    DOC.clear();

    iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC) ? P_readOK_c = true : P_readOK_c = false;
    update_sketch_parameters_flash(DOC);
    DOC.clear();
}

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        status_mqtt(0);
        status_mqtt(1);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2:No other functions");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2:");
        iot.pub_msg(msg);
    }
    else
    {
        if (iot.num_p > 1)
        {
            if (strcmp(iot.inline_param[1], "remain") == 0)
            {
                notifyRemain(atoi(iot.inline_param[0]));
            }
            else if (strcmp(iot.inline_param[1], "off") == 0)
            {
                if (timeoutButtonV[atoi(iot.inline_param[0])]->getState())
                {
                    Ext_trigger_OFF(2, atoi(iot.inline_param[0]));
                }
            }
            else if (strcmp(iot.inline_param[1], "on") == 0)
            {
                if (timeoutButtonV[atoi(iot.inline_param[0])]->getState()) /* turn off if it is ON */
                {
                    Ext_trigger_OFF(2, atoi(iot.inline_param[0]));
                }

                if (iot.num_p == 2)
                {
                    Ext_trigger_ON(2, 0, 0, atoi(iot.inline_param[0])); /* Default Timeout & default PWM value*/
                }
                else if (iot.num_p == 3)
                {
                    Ext_trigger_ON(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0])); /* define Timeout*/
                }
                else if (iot.num_p == 4)
                {
                    Ext_trigger_ON(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[3]), atoi(iot.inline_param[0])); /* define Timeout & PWM*/
                }
            }
            else if (strcmp(iot.inline_param[1], "add") == 0)
            {
                Ext_addTime(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0]));
            }
            else if (strcmp(iot.inline_param[1], "updatePWM") == 0)
            {
                Ext_updatePWM_value(2, atoi(iot.inline_param[2]), atoi(iot.inline_param[0]));
            }
        }
    }
}
void startIOTservices()
{
    update_Parameters_flash();
    iot.start_services(addiotnalMQTT);
}
