#include <myIOT.h>

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
#define USE_EXT_TOPIC true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "espSleepTEST_2"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "SolarPower"
#define MQTT_EXT_TOPIC MQTT_PREFIX "/" MQTT_GROUP "/" DEVICE_TOPIC "/" \
                                   "debug"
myIOT iot;
extern void wait4OTA();

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        // sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
        iot.pub_msg("I'm awake");
    }
    else if (strcmp(incoming_msg, "ver") == 0)
    {
        sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help") == 0)
    {
        sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
        iot.pub_msg(msg);
    }
}
void startIOTservices()
{
    iot.useSerial = USE_SERIAL;
    iot.useWDT = USE_WDT;
    iot.useOTA = USE_OTA;
    iot.useResetKeeper = USE_RESETKEEPER;
    iot.resetFailNTP = USE_FAILNTP;
    iot.useextTopic = USE_EXT_TOPIC;
    strcpy(iot.deviceTopic, DEVICE_TOPIC);
    strcpy(iot.prefixTopic, MQTT_PREFIX);
    strcpy(iot.addGroupTopic, MQTT_GROUP);
    strcpy(iot.extTopic, MQTT_EXT_TOPIC);

    iot.start_services(addiotnalMQTT);
}
void getEXTtopicMqtt(char *msg)
{
    if (strcmp(iot.mqqt_ext_buffer[1], "") != 0 && strcmp(iot.mqqt_ext_buffer[1], msg) == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            strcpy(iot.mqqt_ext_buffer[i], "");
        }
        wait4OTA();
    }
}