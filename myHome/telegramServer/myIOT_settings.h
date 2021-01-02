#include <myIOT2.h>

#define DEV_TOPIC "TelegramServer"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"
#define TELE_MQTT_TOPIC PREFIX_TOPIC "/sms"

myIOT2 iot;
// extern void construct_msgOUT(const char *from, const char *subj, const char *msg, JsonDocument &DOC);
extern void sendMSGtoMQTT(const char *from, const char *subj, const char *msg);
extern const char *mserver_ver;

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
        sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", mserver_ver, iot.ver, isESP8266 ? "ESP8266" : "ESP32");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [; m; ,x]");
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);

        if (strcmp(iot.inline_param[0], "sms") == 0)
        {
            sendMSGtoMQTT(iot.inline_param[3], iot.inline_param[2], iot.inline_param[1]);
        }
        for (int i = 0; i < iot.num_param; i++)
        {
            strcpy(iot.inline_param[i], "");
        }
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = true;
    iot.resetFailNTP = true;
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);
    strcpy(iot.extTopic, TELE_MQTT_TOPIC);

    iot.start_services(addiotnalMQTT);
}
