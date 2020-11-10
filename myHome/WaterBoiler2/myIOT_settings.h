#include <myIOT.h>
extern mySwitch TOswitch;
extern const int TimeOut;
extern const char *clockAlias;
extern void switchOn(char *txt);
extern void switchOff(char *txt);

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "WaterBoiler2"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""

// ********** Sketch Services  ***********
#define VER "Wemos_7.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true      // Serial Monitor
#define USE_WDT true         // watchDog resets
#define USE_OTA true         // OTA updates
#define USE_RESETKEEPER true // detect quick reboot and real reboots
#define USE_FAILNTP true     // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

myIOT iot;
// *********************
void all_off(char *from)
{
    TOswitch.all_off(from);
    TOswitch.TOswitch.updateStart(0);
}
void giveStatus(char *outputmsg)
{
    char t1[50];
    char t2[50];
    char t3[150];

    sprintf(t3, "Status: ");
    if (TOswitch.TOswitch.remain() > 0)
    {
        TOswitch.TOswitch.convert_epoch2clock(now() + TOswitch.TOswitch.remain(), now(), t2, t1);
        sprintf(t1, "timeLeft[%s]", t2);
    }
    else
    {
        sprintf(t1, "");
    }
    sprintf(t2, "[%s] [%s] %s ", "SW_Names", TOswitch.current_power ? "ON" : "OFF", t1);
    strcat(t3, t2);
    sprintf(outputmsg, "%s", t3);
}
void addiotnalMQTT(char *income_msg)
{
    char msg_MQTT[150];
    char msg2[20];

    if (strcmp(income_msg, "status") == 0)
    {
        giveStatus(msg_MQTT);
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "ver") == 0)
    {
        sprintf(msg_MQTT, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
        iot.pub_msg(msg_MQTT);
        // sprintf(msg_MQTT, "ver #2: DailyTO[%d], ON_AT_BOOT[%d], HardReboot[%d]", USE_DAILY_TO, ON_AT_BOOT, HARD_REBOOT);
        // iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "help") == 0)
    {
        sprintf(msg_MQTT, "Help: Commands #1 - [on, off, flash, format]");
        iot.pub_msg(msg_MQTT);
        sprintf(msg_MQTT, "Help: Commands #2 - [remain, restartTO, timeout(x), endTO, updateTO(x), restoreTO, statusTO]");
        iot.pub_msg(msg_MQTT);
        sprintf(msg_MQTT, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
        iot.pub_msg(msg_MQTT);
        sprintf(msg_MQTT, "Help: Commands #4 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
        iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(income_msg, "flash") == 0)
    {
        TOswitch.TOswitch.inCodeTimeOUT_inFlash.printFile();
    }
    else if (strcmp(income_msg, "format") == 0)
    {
        TOswitch.TOswitch.inCodeTimeOUT_inFlash.format();
    }
    else if (strcmp(income_msg, "all_off") == 0)
    {
        all_off("MQTT");
    }
    else if (strcmp(income_msg, "on") == 0)
    {
        switchOn("MQTT");
    }
    else if (strcmp(income_msg, "off") == 0)
    {
        switchOff("MQTT");
    }
    else
    {
        int num_parameters = iot.inline_read(income_msg);

        if (strcmp(iot.inline_param[0], "timeout") == 0)
        {
            TOswitch.TOswitch.setNewTimeout(atoi(iot.inline_param[1]));
            TOswitch.TOswitch.convert_epoch2clock(now() + atoi(iot.inline_param[1]) * 60, now(), msg2, msg_MQTT);
            TOswitch.TOswitch.updateStart(now());
            sprintf(msg_MQTT, "TimeOut: one-time TimeOut %s", msg2);
            iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(iot.inline_param[0], "updateTO") == 0)
        {
            TOswitch.TOswitch.updateTOinflash(atoi(iot.inline_param[1]));
            sprintf(msg_MQTT, "TimeOut: Updated in flash to [%d min.]", atoi(iot.inline_param[1]));
            iot.pub_msg(msg_MQTT);
            delay(1000);
            iot.notifyOffline();
            iot.sendReset("TimeOut update");
        }
        else if (strcmp(iot.inline_param[0], "remain") == 0)
        {
            if (TOswitch.TOswitch.remain() > 0)
            {
                TOswitch.TOswitch.convert_epoch2clock(now() + TOswitch.TOswitch.remain(), now(), msg2, msg_MQTT);
                sprintf(msg_MQTT, "TimeOut: Remain [%s]", msg2);
                iot.pub_msg(msg_MQTT);
            }
        }
        else if (strcmp(iot.inline_param[0], "restartTO") == 0)
        {
            TOswitch.TOswitch.restart_to();
            TOswitch.TOswitch.updateStart(now());
            sprintf(msg_MQTT, "TimeOut: [Restart]");
            iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(iot.inline_param[0], "statusTO") == 0)
        {
            sprintf(msg_MQTT, "%s: {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}","TimeOut", TimeOut, 
            TOswitch.TOswitch.updatedTO_inFlash,TOswitch.TOswitch.updatedTO_inFlash ? "Flash" : "inCode");
            iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(iot.inline_param[0], "endTO") == 0)
        {
            TOswitch.TOswitch.endNow();
            TOswitch.TOswitch.updateStart(0);
            sprintf(msg_MQTT, "TimeOut: [Abort]");
            iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(iot.inline_param[0], "restoreTO") == 0)
        {
            TOswitch.TOswitch.restore_to();
            TOswitch.TOswitch.restart_to();
            TOswitch.TOswitch.updateStart(now());
            sprintf(msg_MQTT, "TimeOut: Restore hardCoded Value [%d mins.]", TimeOut);
            iot.pub_msg(msg_MQTT);
            iot.notifyOffline();
            iot.sendReset("Restore");
        }
        // else if (strcmp(iot.inline_param[0], "on_dailyTO") == 0)
        // {
        //     TOswitch.TOswitch.dailyTO.on[0] = atoi(iot.inline_param[1]); //hours
        //     TOswitch.TOswitch.dailyTO.on[1] = atoi(iot.inline_param[2]); // minutes
        //     TOswitch.TOswitch.dailyTO.on[2] = atoi(iot.inline_param[3]); // seconds

        //     TOswitch.TOswitch.store_dailyTO_inFlash(TOswitch.TOswitch.dailyTO, atoi(iot.inline_param[0]));

        //     sprintf(msg_MQTT, "%s: [ON] updated [%02d:%02d:%02d]", clockAlias,
        //             TOswitch.TOswitch.dailyTO.on[0], TOswitch.TOswitch.dailyTO.on[1],
        //             TOswitch.TOswitch.dailyTO.on[2]);

        //     iot.pub_msg(msg_MQTT);
        // }
        // else if (strcmp(iot.inline_param[0], "off_dailyTO") == 0)
        // {
        //     TOswitch.TOswitch.dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
        //     TOswitch.TOswitch.dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
        //     TOswitch.TOswitch.dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

        //     TOswitch.TOswitch.store_dailyTO_inFlash(TOswitch.TOswitch.dailyTO, atoi(iot.inline_param[0]));

        //     sprintf(msg_MQTT, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
        //             TOswitch.TOswitch.dailyTO.off[0], TOswitch.TOswitch.dailyTO.off[1],
        //             TOswitch.TOswitch.dailyTO.off[2]);

        //     iot.pub_msg(msg_MQTT);
        // }
        // else if (strcmp(iot.inline_param[0], "flag_dailyTO") == 0)
        // {
        //     TOswitch.TOswitch.dailyTO.flag = atoi(iot.inline_param[2]);
        //     TOswitch.TOswitch.store_dailyTO_inFlash(TOswitch.TOswitch.dailyTO, atoi(iot.inline_param[0]));

        //     sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
        //             atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

        //     iot.pub_msg(msg_MQTT);
        // }
        // else if (strcmp(iot.inline_param[0], "useflash_dailyTO") == 0)
        // {
        //     TOswitch.TOswitch.dailyTO.useFlash = atoi(iot.inline_param[2]);
        //     TOswitch.TOswitch.store_dailyTO_inFlash(TOswitch.TOswitch.dailyTO, atoi(iot.inline_param[0]));

        //     sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias, atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

        //     iot.pub_msg(msg_MQTT);
        // }
        // else if (strcmp(iot.inline_param[0], "status_dailyTO") == 0)
        // {
        //     sprintf(msg_MQTT, "%s: Switch [#%d] {ON:%02d:%02d:%02d} {OFF:%02d:%02d:%02d} {Flag:%s} {Values:%s}",
        //             clockAlias, atoi(iot.inline_param[0]),
        //             TOswitch.TOswitch.dailyTO.on[0],
        //             TOswitch.TOswitch.dailyTO.on[1],
        //             TOswitch.TOswitch.dailyTO.on[2],
        //             TOswitch.TOswitch.dailyTO.off[0],
        //             TOswitch.TOswitch.dailyTO.off[1],
        //             TOswitch.TOswitch.dailyTO.off[2],
        //             TOswitch.TOswitch.dailyTO.flag ? "ON" : "OFF",
        //             TOswitch.TOswitch.dailyTO.useFlash ? "Flash" : "inCode");
        //     iot.pub_msg(msg_MQTT);

        //     Serial.println("IN STATUS_Daily LOOP");
        // }
        // else if (strcmp(iot.inline_param[0], "restart_dailyTO") == 0)
        // {
        //     TOswitch.TOswitch.restart_dailyTO(TOswitch.TOswitch.dailyTO);
        //                 TOswitch.TOswitch.updateStart(now());
        //     sprintf(msg_MQTT, "%s: Switch[#%d] Resume daily Timeout", clockAlias, atoi(iot.inline_param[0]));
        //     iot.pub_msg(msg_MQTT);
        // }

        else
        {
            if (strcmp(income_msg, "offline") != 0 && strcmp(income_msg, "online") != 0)
            {
                sprintf(msg_MQTT, "Unrecognized Command: [%s]", income_msg);
                iot.pub_log(msg_MQTT);
            }
        }
        for (int n = 0; n <= num_parameters - 1; n++)
        {
            sprintf(iot.inline_param[n], "");
        }
    }
}
void startIOTservices()
{
    iot.useSerial = USE_SERIAL;
    iot.useWDT = USE_WDT;
    iot.useOTA = USE_OTA;
    iot.useResetKeeper = USE_RESETKEEPER;
    iot.resetFailNTP = USE_FAILNTP;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    strcpy(iot.deviceTopic, DEVICE_TOPIC);
    strcpy(iot.prefixTopic, MQTT_PREFIX);
    strcpy(iot.addGroupTopic, MQTT_GROUP);
    iot.start_services(addiotnalMQTT);
}