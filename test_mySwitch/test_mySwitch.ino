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
#define USE_TELEGRAM false
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "testNEWlib"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define TELEGRAM_OUT_TOPIC "Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// *********** myTOswitch ***********

// ~~~~ Daily TO ~~~~~~
const int START_dTO[] = {13, 5, 30};
const int END_dTO[] = {13, 6, 0};
// ~~~~~~~~~~~~~~~~~~~~

const int TimeOUT = 2; // minutes after pressing ON
const int PWMPin = D3;
const int ledPin = D4;
char msgtoMQTT[150];

mySwitch myTOsw(PWMPin, TimeOUT);

void startTOSwitch()
{
        myTOsw.usePWM = true;
        myTOsw.useSerial = USE_SERIAL;
        myTOsw.useInput = true;
        myTOsw.badBoot = true;
        myTOsw.useDailyTO = true;
        myTOsw.useEXTtrigger = true;
        myTOsw.is_momentery = true;
        myTOsw.inputPin = D7;

        if (myTOsw.useDailyTO)
        {
                for (int i = 0; i < 3; i++)
                {
                        myTOsw.START_dailyTO[i] = START_dTO[i];
                        myTOsw.END_dailyTO[i] = END_dTO[i];
                }
        }
        myTOsw.extTrig_cb(HIGH,false,"GUYZ_fake");

        myTOsw.begin();
}
void TOswitch_looper()
{
        myTOsw.looper(iot.mqtt_detect_reset);
        if (myTOsw.postMessages(msgtoMQTT))
        {
                iot.pub_msg(msgtoMQTT);
        }
}
void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        iot.useTelegram = USE_TELEGRAM;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        strcpy(iot.telegramServer, TELEGRAM_OUT_TOPIC);
        iot.start_services(ADD_MQTT_FUNC);
}
// ***********************************
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                sprintf(msg, "Status: Im OK");
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
        // myTOsw.TOswitch.setNewTimeout(5);
}
void loop()
{
        iot.looper();
        TOswitch_looper();
        myTOsw.ext_trig_signal = digitalRead(D2);

        delay(100);
}
