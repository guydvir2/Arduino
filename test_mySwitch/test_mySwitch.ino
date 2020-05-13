#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "testNEWlib"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

const int START_dTO[] = {16, 0, 0};
const int END_dTO[] = {0, 30, 0};

mySwitch myTOsw(D3, "MyLove-one", 2);

void startTOSwitch()
{
        myTOsw.usePWM = true;
        myTOsw.useSerial = true;
        myTOsw.useInput = true;
        myTOsw.badBoot = true;
        myTOsw.useDailyTO = true;
        myTOsw.inputPin = D7;
        for (int i = 0; i < 3; i++)
        {
                myTOsw.START_dailyTO[i] = START_dTO[i];
                myTOsw.END_dailyTO[i] = END_dTO[i];
        }

        myTOsw.begin();
}

void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                // sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
                iot.pub_msg(msg);
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

void setup()
{
        startIOTservices();
        startTOSwitch();
        myTOsw.TOswitch.remain();
}
void loop()
{

        iot.looper();
        myTOsw.looper(iot.mqtt_detect_reset);
        delay(100);
}
