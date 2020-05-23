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
#define NUM_SW 2
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
        else
        {
                int num_parameters = iot.inline_read(incoming_msg);
                TOswitches[atoi(iot.inline_param[0])]->getMQTT(iot.inline_param[1], atoi(iot.inline_param[2]), atoi(iot.inline_param[3]), atoi(iot.inline_param[4]));
        }

        //         if (strcmp(iot.inline_param[1], "on") == 0)
        //         {
        //                 if (USE_PWM)
        //                 {
        //                         // switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false, def_power);
        //                 }
        //                 else
        //                 {
        //                         // switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false);
        //                 }
        //         }
        //         else if (strcmp(iot.inline_param[1], "off") == 0)
        //         {
        //                 if (USE_PWM)
        //                 {
        //                         // switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false, 0.0);
        //                 }
        //                 else
        //                 {
        //                         // switchIt("MQTT", atoi(io;[t.inline_param[0]), 0, "", false);
        //                 }
        //         }
        //         else if (strcmp(iot.inline_param[1], "timeout") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->TOswitch.setNewTimeout(atoi(iot.inline_param[2]));
        //                 TOswitches[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg_MQTT);
        //                 sprintf(msg_MQTT, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "updateTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
        //                 sprintf(msg_MQTT, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
        //                 iot.pub_msg(msg_MQTT);
        //                 delay(1000);
        //                 iot.notifyOffline();
        //                 iot.sendReset("TimeOut update");
        //         }
        //         else if (strcmp(iot.inline_param[1], "remain") == 0)
        //         {
        //                 if (TOswitches[atoi(iot.inline_param[0])]->remain() > 0)
        //                 {
        //                         TOswitches[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TOswitches[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg_MQTT);
        //                         sprintf(msg_MQTT, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
        //                         iot.pub_msg(msg_MQTT);
        //                 }
        //         }
        //         else if (strcmp(iot.inline_param[1], "restartTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->restart_to();
        //                 sprintf(msg_MQTT, "TimeOut: Switch [#%d] [Restart]", atoi(iot.inline_param[0]));
        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "statusTO") == 0)
        //         {
        //                 sprintf(msg_MQTT, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
        //                         "TimeOut", atoi(iot.inline_param[0]),
        //                         TIMEOUTS[atoi(iot.inline_param[0])],
        //                         TOswitches[atoi(iot.inline_param[0])]->updatedTO_inFlash,
        //                         TOswitches[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode");
        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "endTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->endNow();
        //                 sprintf(msg_MQTT, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "restoreTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->restore_to();
        //                 TOswitches[atoi(iot.inline_param[0])]->restart_to();
        //                 sprintf(msg_MQTT, "TimeOut: Switch [#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
        //                 iot.pub_msg(msg_MQTT);
        //                 iot.notifyOffline();
        //                 iot.sendReset("Restore");
        //         }
        //         else if (strcmp(iot.inline_param[1], "on_dailyTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[0] = atoi(iot.inline_param[2]); //hours
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[1] = atoi(iot.inline_param[3]); // minutes
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[2] = atoi(iot.inline_param[4]); // seconds

        //                 TOswitches[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TOswitches[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

        //                 sprintf(msg_MQTT, "%s: Switch [#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[0], TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[1],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[2]);

        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "off_dailyTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

        //                 TOswitches[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TOswitches[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

        //                 sprintf(msg_MQTT, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[0], TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[1],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[2]);

        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "flag_dailyTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.flag = atoi(iot.inline_param[2]);
        //                 TOswitches[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TOswitches[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

        //                 sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
        //                         atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "useflash_dailyTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->dailyTO.useFlash = atoi(iot.inline_param[2]);
        //                 TOswitches[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TOswitches[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

        //                 sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
        //                         atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "status_dailyTO") == 0)
        //         {
        //                 sprintf(msg_MQTT, "%s: Switch [#%d] {ON:%02d:%02d:%02d} {OFF:%02d:%02d:%02d} {Flag:%s} {Values:%s}",
        //                         clockAlias, atoi(iot.inline_param[0]),
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[0],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[1],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.on[2],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[0],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[1],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.off[2],
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.flag ? "ON" : "OFF",
        //                         TOswitches[atoi(iot.inline_param[0])]->dailyTO.useFlash ? "Flash" : "inCode");
        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "restart_dailyTO") == 0)
        //         {
        //                 TOswitches[atoi(iot.inline_param[0])]->restart_dailyTO(TOswitches[atoi(iot.inline_param[0])]->dailyTO);
        //                 sprintf(msg_MQTT, "%s: Switch[#%d] Resume daily Timeout", clockAlias, atoi(iot.inline_param[0]));
        //                 iot.pub_msg(msg_MQTT);
        //         }
        //         else if (strcmp(iot.inline_param[1], "change_pwm") == 0)
        //         {
        //                 switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false, atoi(iot.inline_param[2]));
        //         }

        //         else
        //         {
        //                 if (strcmp(incoming_msg, "offline") != 0 && strcmp(incoming_msg, "online") != 0 && strcmp(incoming_msg, "resetKeeper") != 0)
        //                 {
        //                         sprintf(msg_MQTT, "Unrecognized Command: [%s]", incoming_msg);
        //                         iot.pub_log(msg_MQTT);
        //                 }
        //         }
        //         for (int n = 0; n <= num_parameters - 1; n++)
        //         {
        //                 sprintf(iot.inline_param[n], "");
        //         }
        // }
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

