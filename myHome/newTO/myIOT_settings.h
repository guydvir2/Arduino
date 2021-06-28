#include <myIOT2.h>
#include <myTimeoutSwitch.h>

myIOT2 iot;

#define DEV_TOPIC "empty"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"

extern timeOUTSwitch timeoutSW_0;

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        bool Q = digitalRead(OUTPUT_PIN);
        sprintf(msg, "OUTPUT IS: %d", Q);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
        // iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [on, off,{timeout,_sec}]");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "on") == 0)
    {
        timeoutSW_0.start_TO(timeoutSW_0.maxON_minutes, "MQTT");
    }
    else if (strcmp(incoming_msg, "off") == 0)
    {
        int t = timeoutSW_0.remTime();
        timeoutSW_0.finish_TO("MQTT");
    }
    // else if (strcmp(incoming_msg, "switch") == 0)
    // {
    //     digitalWrite(INPUT_PIN, !digitalRead(INPUT_PIN));
    //     sprintf(msg, "INPUT IS: %d", digitalRead(INPUT_PIN));
    //     iot.pub_msg(msg);
    // }
    // else if (strcmp(incoming_msg, "press") == 0)
    // {
    //     digitalWrite(INPUT_PIN, LOW);
    //     delay(100);
    //     digitalWrite(INPUT_PIN, HIGH);
    //     sprintf(msg, "PRESS ON & OFF");
    //     iot.pub_msg(msg);
    // }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (num_parameters > 1)
        {
            if (strcmp(iot.inline_param[0], "timeout") == 0)
            {
                timeoutSW_0.start_TO(atoi(iot.inline_param[1]), "MQTT");
            }
        }
    }
}
void startIOTservices()
{
#if USE_SIMPLE_IOT == 1
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = false;
    iot.resetFailNTP = true;
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

#elif USE_SIMPLE_IOT == 0

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
    iot.useBootClockLog = true;
    strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
    strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
    strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
#endif

    // char a[50];
    // sprintf(a, "%s/%s/%s/%s", iot.prefixTopic, iot.addGroupTopic, iot.deviceTopic, DEBUG_TOPIC);
    // strcpy(iot.extTopic, a);

    iot.start_services(addiotnalMQTT);
    // iot.start_services(addiotnalMQTT, "dvirz_iot", "GdSd13100301", MQTT_USER, MQTT_PASS, "192.168.2.100");
}
