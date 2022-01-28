#include <myIOT2.h>

myIOT2 iot;

#define DEV_TOPIC "indic"
#define GROUP_TOPIC "TFTscreens"
#define PREFIX_TOPIC "myHome"

// MQTT_msg extTopic_msg; /* ExtTopic*/

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "BOOOOO");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
        // iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "disco") == 0)
    {
        sprintf(msg, "Disconnect sent to broker");
        iot.pub_msg(msg);
        iot.mqttClient.disconnect();
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = true;
    iot.useextTopic = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 10;
    iot.useBootClockLog = false;
    iot.useAltermqttServer = false;
    iot.ignore_boot_msg = false;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);
    // iot.extTopic_msgArray[0] = &extTopic_msg;
    // iot.extTopic[0] = "myHome/new";
    iot.start_services(addiotnalMQTT);
}