#include <myIOT.h>
#include "empty_param.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_3.0"
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;

void startIOTservices()
{
        iot.useSerial = paramJSON["useSerial"];
        iot.useWDT = paramJSON["useWDT"];
        iot.useOTA = paramJSON["useOTA"];
        iot.useResetKeeper = paramJSON["useResetKeeper"];
        iot.resetFailNTP = paramJSON["useFailNTP"];
        iot.useDebug = paramJSON["useDebugLog"];
        iot.debug_level = paramJSON["debug_level"];
        strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
        strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
        strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
        iot.start_services(ADD_MQTT_FUNC);
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                sprintf(msg, "Status: OK");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
                sprintf(msg, "Help: Commands #3 - [NEW]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                sprintf(msg, "Ver: Ver:%s", VER);
                iot.pub_msg(msg);
        }
}

void setup()
{
        startIOTservices();
}
void loop()
{
        iot.looper();
        delay(100);
}
