#include <myIOT.h>
#include <myPIR.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.1"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false       // Serial Monitor
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

// ****************** PIR Detector **************
const int PIRPin = D2;
PIRsensor PIR_0(PIRPin, "PIR_detector", 15, HIGH);
bool inDetection = false;
void startPIRservices()
{
        PIR_0.use_serial = USE_SERIAL;
        PIR_0.ignore_det_interval = 10;
        PIR_0.delay_first_detection = 5;
        PIR_0.use_timer = false;
        PIR_0.timer_duration = 10;

        // bool stop_sensor = false; // during code run, select to disable sensor activity
        PIR_0.trigger_once = true;
        PIR_0.start();
}
void PIRlooper()
{
        PIR_0.looper();
        // inDetection = PIR_0.logic_state;
        if (PIR_0.logic_state && inDetection == false)
        {
                inDetection = true;
                Serial.println("DETECTION");
        }
        else if (PIR_0.logic_state == false && inDetection)
        {
                inDetection = false;
                Serial.println("END_DETECTION");
        }
}

// *********** myTOswitch ***********

// ~~~~ TO & dailyTO ~~~~~~
#define USE_TO true
#define USE_dailyTO true
const int START_dTO[2][3] = {{12, 55, 0}, {20, 10, 0}};
const int END_dTO[2][3] = {{12, 55, 30}, {20, 30, 0}};
const int TimeOUT[] = {1, 1}; // minutes
// ~~~~~~~~~~~~~~~~~~~~

// ~~~~~~ Hardware ~~~~~~~
#define NUM_SW 2
#define USE_PWM true
#define USE_INPUT true
#define USE_EXT_TRIG true
#define BUTTOM_MOMENT true
const int outputPin[] = {D3, D4};
const int inputPin[] = {D5, D6};
char *SW_Names[] = {"Switch_A", "Switch_B"};
// ~~~~~~~~~~~~~~~~~~~~~~~

mySwitch myTOsw0(outputPin[0], TimeOUT[0], SW_Names[0]);
#if NUM_SW == 2
mySwitch myTOsw1(outputPin[1], TimeOUT[1], SW_Names[1]);
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
                TOswitches[i]->badBoot = true;
                TOswitches[i]->usetimeOUT = USE_TO;
                TOswitches[i]->useDailyTO = USE_dailyTO;

                TOswitches[i]->inputPin = inputPin[i];

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
        for (int i = 0; i < NUM_SW; i++)
        {
                TOswitches[i]->looper(iot.mqtt_detect_reset);
                TOswitches[i]->ext_trig_signal=PIR_0.logic_state;
                
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
        startIOTservices();
        startTOSwitch();
        startPIRservices();
}
void loop()
{
        iot.looper();
        TOswitch_looper();
        PIRlooper();
        delay(100);
}
