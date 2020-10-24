#include <myIOT.h>
#include "radar_param.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_3.1"
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
        iot.useNetworkReset = paramJSON["useNetworkReset"];
        iot.noNetwork_reset = paramJSON["noNetwork_reset"];
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
void radarScan(int sensPin=D2)
{
        static long firstDetect_clock = 0;
        static bool detect = false;
        const int SEC_TO_APPROVE_DETECT = 4;

        if (detect == false && digitalRead(sensPin) == true)
        {
                if (firstDetect_clock == 0)
                {
                        firstDetect_clock = millis();
                        Serial.println("gong");
                }
                else if (millis() - firstDetect_clock > 1000 * SEC_TO_APPROVE_DETECT)
                {
                        Serial.println("Detect");
                        detect = true;
                }
        }
        else if (detect == false && digitalRead(sensPin) == false && firstDetect_clock > 1000 * SEC_TO_APPROVE_DETECT)
        {
                firstDetect_clock = 0;
                Serial.println("Zero- no activity");
        }
        else if (detect == true && digitalRead(sensPin) == false)
        {
                detect = false;
                firstDetect_clock = 0;
                Serial.println("END_Detect");
        }
}

void setup()
{
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
}
void loop()
{
        iot.looper();
        radarScan();
        delay(100);
}
