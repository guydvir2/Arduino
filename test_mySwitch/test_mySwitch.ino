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

// ~~~~ TO & dailyTO ~~~~~~
#define USE_TO true
#define USE_dailyTO true
const int START_dTO[2][3] = {{16, 0, 0}, {16, 0, 0}};
const int END_dTO[2][3] = {{0, 30, 0}, {0, 30, 0}};
const int TimeOUT[] = {120, 120}; // minutes
// ~~~~~~~~~~~~~~~~~~~~

// ~~~~~~ Hardware ~~~~~~~
#define NUM_SW 1
#define USE_PWM true
#define USE_INPUT true
#define USE_EXT_TRIG false
#define BUTTOM_MOMENT true
const int PWMPin[] = {D3, D3};
const int inputPin[] = {D7, D7};
// const int PIRPin = D4;
// ~~~~~~~~~~~~~~~~~~~~~~~

mySwitch myTOsw0(PWMPin[0], TimeOUT[0]);
mySwitch myTOsw1(PWMPin[1], TimeOUT[1]);
#if NUM_SW == 2
mySwitch *TOswitches[NUM_SW] = {&myTOsw0, &myTOsw1};
#elif NUM_SW == 1
mySwitch *TOswitches[NUM_SW] = {&myTOsw0};
#endif

void startTOSwitch()
{
        for (int i = 0; i < NUM_SW; i++)
        {
                TOswitches[i]->usePWM = USE_PWM;
                TOswitches[i]->useSerial = USE_SERIAL;
                TOswitches[i]->useInput = USE_INPUT;
                TOswitches[i]->useEXTtrigger = USE_EXT_TRIG;
                TOswitches[i]->is_momentery = BUTTOM_MOMENT;
                TOswitches[i]->inputPin = inputPin[i];
                TOswitches[i]->badBoot = true;
                TOswitches[i]->usetimeOUT = USE_TO;
                TOswitches[i]->useDailyTO = USE_dailyTO;

                if (TOswitches[i]->useDailyTO)
                {
                        TOswitches[i]->setdailyTO(START_dTO[i], END_dTO[i]);
                }
                if (TOswitches[i]->useEXTtrigger)
                {
                        TOswitches[i]->extTrig_cb(HIGH, false, "PIR_DETECTOR");
                }

                TOswitches[i]->begin();
        }
}
void TOswitch_looper()
{
        char msgtoMQTT[150];
        for (int i = 0; i < NUM_SW; i++)
        {
                TOswitches[i]->looper(iot.mqtt_detect_reset);
                if (TOswitches[i]->postMessages(msgtoMQTT))
                {
                        iot.pub_msg(msgtoMQTT);
                }
        }
}
// ***********************************

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
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d], Telegram[%d]",
                        VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP, USE_TELEGRAM);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0)
        {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "flash") == 0)
        {
                myTOsw0.TOswitch.inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(incoming_msg, "format") == 0)
        {
                myTOsw0.TOswitch.inCodeTimeOUT_inFlash.format();
        }
        else if (strcmp(incoming_msg, "all_off") == 0)
        {
                // all_off("MQTT");
        }
}

void setup()
{
        startIOTservices();
        startTOSwitch();
}
void loop()
{
        iot.looper();
        TOswitch_looper();

        delay(100);
}
