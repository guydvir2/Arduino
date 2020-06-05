#include <myIOT.h>
#include <myPIR.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "SonOff_1.3"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true         // watchDog resets
#define USE_OTA true         // OTA updates
#define USE_RESETKEEPER true // detect quick reboot and real reboots
#define USE_FAILNTP true     // saves amoount of fail clock updates
#define USE_TELEGRAM false
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "PergolaBulbs"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "extLights"
#define TELEGRAM_OUT_TOPIC "Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// *********** myTOswitch ***********
// ~~~~~~ Services ~~~~~~~~
#define ON_AT_BOOT false
#define USE_QUICK_BOOT true
#define USE_TO true
#define USE_dailyTO true
#define SAFETY_OFF true
#define SAFEY_OFF_DURATION 600 //minutes
#define USE_BADBOOT USE_RESETKEEPER
#define USE_EEPROM_RESET_COUNTER true
// ~~~~~~~~~~~~~~~~~~~~

// ~~~~ TO & dailyTO ~~~~~~
const int START_dTO[2][3] = {{23, 30, 0}, {20, 30, 0}};
const int END_dTO[2][3] = {{6, 15, 0}, {22, 0, 0}};
const int TimeOUT[] = {240, 1}; // minutes
// ~~~~~~~~~~~~~~~~~~~~

// ~~~~~~ Hardware ~~~~~~~
#define NUM_SW 1
#define USE_PWM false
#define USE_INPUT false
#define USE_EXT_TRIG false
#define BUTTOM_MOMENT true

const int outputPin[] = {12, 5}; // D3 for most PWM boards
const int inputPin[] = {0, 0};
const int hRebbots[]={1,2};
// ~~~~~~~~~~~~~~~~~~~~
char *SW_Names[] = {"Lamps", "Strips"};

/*
~~~~~ SONOFF HARDWARE ~~~~~
#define RELAY1 12
#define RELAY2 5
#define INPUT1 0  // 0 for onBoard Button
#define INPUT2 14 // 14 for extButton
#define indic_LEDpin 13
*/
// ~~~~~~~~~~~~~~~~~~~~~~~

mySwitch myTOsw0(outputPin[0], TimeOUT[0], SW_Names[0]);
#if NUM_SW == 2
mySwitch myTOsw1(outputPin[1], TimeOUT[1], SW_Names[1]);
mySwitch *TOswitches[NUM_SW] = {&myTOsw0, &myTOsw1};
#elif NUM_SW == 1
mySwitch *TOswitches[NUM_SW] = {&myTOsw0};
#endif

void configTOswitches()
{
        for (int i = 0; i < NUM_SW; i++)
        {
                TOswitches[i]->usePWM = USE_PWM;
                TOswitches[i]->useSerial = USE_SERIAL;
                TOswitches[i]->useInput = USE_INPUT;
                TOswitches[i]->useEXTtrigger = USE_EXT_TRIG;
                TOswitches[i]->useHardReboot = USE_EEPROM_RESET_COUNTER;
                TOswitches[i]->is_momentery = BUTTOM_MOMENT;
                TOswitches[i]->badBoot = USE_BADBOOT;
                TOswitches[i]->useDailyTO = USE_dailyTO;
                TOswitches[i]->usesafetyOff = SAFETY_OFF;
                TOswitches[i]->set_safetyoff = SAFEY_OFF_DURATION;
                TOswitches[i]->usequickON = USE_QUICK_BOOT;
                TOswitches[i]->onAt_boot = ON_AT_BOOT;
                TOswitches[i]->inputPin = inputPin[i];
                
                if (USE_EEPROM_RESET_COUNTER)
                {
                        TOswitches[i]->hReboot.check_boot(hRebbots[i]);
                }
                if (USE_QUICK_BOOT)
                {
                        TOswitches[i]->quickPwrON();
                }
        }
}
void startTOSwitch()
{
        // After Wifi is On
        for (int i = 0; i < NUM_SW; i++)
        {
                if (TOswitches[i]->useDailyTO)
                {
                        TOswitches[i]->setdailyTO(START_dTO[i], END_dTO[i]);
                }
                if (TOswitches[i]->useEXTtrigger)
                {
                        TOswitches[i]->extTrig_cb(HIGH, true, "PIR_DETECTOR");
                }
                TOswitches[i]->begin();
        }
}
void TOswitch_looper()
{
        char msgtoMQTT[150];
        byte mtyp;

        for (int i = 0; i < NUM_SW; i++)
        {
                TOswitches[i]->looper(iot.mqtt_detect_reset);
                if (TOswitches[i]->postMessages(msgtoMQTT, mtyp))
                {
                        if (mtyp == 0)
                        {
                                iot.pub_msg(msgtoMQTT);
                        }
                        else if (mtyp == 1)
                        {
                                iot.pub_log(msgtoMQTT);
                        }
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
void giveStatus(char *outputmsg)
{
        char t1[50];
        char t2[50];
        char t3[150];

        sprintf(t3, "Status: ");
        for (int i = 0; i < NUM_SW; i++)
        {
                if (TOswitches[i]->TOswitch.remain() > 0)
                {
                        TOswitches[i]->TOswitch.convert_epoch2clock(now() + TOswitches[i]->TOswitch.remain(), now(), t2, t1);
                        sprintf(t1, "timeLeft[%s]", t2);
                }
                else
                {
                        sprintf(t1, "");
                }
                if (USE_PWM)
                {
                        if (TOswitches[i]->current_power == 0)
                        {
                                sprintf(t2, "[%s] [OFF] %s ", SW_Names[i], t1);
                        }
                        else
                        {
                                sprintf(t2, "[%s] power[%.0f%%] %s ", SW_Names[i], TOswitches[i]->current_power * 100, t1);
                        }
                }
                else
                {
                        sprintf(t2, "[%s] [%s] %s ", SW_Names[i], TOswitches[i]->current_power ? "ON" : "OFF", t1);
                }
                strcat(t3, t2);
        }
        sprintf(outputmsg, "%s", t3);
}
void all_off(char *from)
{
        for (int i = 0; i < NUM_SW; i++)
        {
                TOswitches[i]->all_off(from);
        }
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];

        if (strcmp(incoming_msg, "status") == 0)
        {
                giveStatus(msg);
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
                sprintf(msg, "Help: Commands #2 - [remain, restartTO, timeout, endTO, updateTO, restoreTO, statusTO]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #4 - [on, off, change_pwm, all_off, flash, format]");
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
                all_off("MQTT");
        }

        // ±±±±±±±±±± MQTT MSGS from mySwitch Library ±±±±±±±±±±±±
        else
        {
                int num_parameters = iot.inline_read(incoming_msg);
                TOswitches[atoi(iot.inline_param[0])]->getMQTT(iot.inline_param[1], atoi(iot.inline_param[2]), atoi(iot.inline_param[3]), atoi(iot.inline_param[4]));
                for (int n = 0; n <= num_parameters - 1; n++)
                {
                        sprintf(iot.inline_param[n], "");
                }
        }
        // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
}

void setup()
{
        configTOswitches();
        startIOTservices();
        startTOSwitch();
}
void loop()
{
        iot.looper();
        TOswitch_looper();
        delay(100);
}
