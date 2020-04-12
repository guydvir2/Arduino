#include <myIOT.h>
#include <myPIR.h>
#include <Arduino.h>

/*
on ESP-01
TX - GPIO1 --> OUTPUT ONLY
RX - GPIO3 --> INPUT  ONLY
*/

// ********** Sketch Services  ***********
#define VER "ESP-01_1.3"
#define Pin_Sensor_0 0
#define Pin_Sensor_1 13 // fake io - not using sensor
#define Pin_Switch_0 2
#define Pin_Switch_1 1
#define Pin_extbut_1 3 // using button to switch on/ off

#define SwitchTimeOUT 30

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "parentsClosetLEDs"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "intLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

#define NUM_SW 2
SensorSwitch s0(Pin_Sensor_0, Pin_Switch_0, SwitchTimeOUT);
SensorSwitch s1(Pin_Sensor_1, Pin_Switch_1, SwitchTimeOUT, Pin_extbut_1);
SensorSwitch *s[NUM_SW] = {&s0, &s1};

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
                sprintf(msg, "Status:");
                for (int i = 0; i < NUM_SW; i++)
                {
                        if (s[i]->swState < 1.0 && s[i]->swState > 0.0)
                        {
                                sprintf(msg2, "LedStrip [%.1f%%] [On] ", s[i]->swState * 100);
                        }
                        else
                        {
                                sprintf(msg2, "LedStrip [#%d] [%s] ", i, s[i]->swState ? "On" : "Off");
                        }
                        strcat(msg, msg2);
                }

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
                sprintf(msg, "Help: Commands #2 - [all_off; i,on; i,off; remain]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "all_off") == 0)
        {
                for (int i = 0; i < NUM_SW; i++)
                {
                        if (s[i]->swState != 0.0)
                        {
                                s[i]->turnOff();
                                sprintf(msg, "MQTT: LedStrip [#%d] Turned [Off]", i);
                                iot.pub_msg(msg);
                        }
                }
        }
        else if (strcmp(incoming_msg, "remain") == 0)
        {
                for (int i = 0; i < NUM_SW; i++)
                {
                        if (s[i]->timeoutRem > 0)
                        {
                                sprintf(msg, "MQTT: Remain Time LedStrip [#%d] ,[%d] sec", i, s[i]->timeoutRem);
                                iot.pub_msg(msg);
                        }
                        else
                        {
                                sprintf(msg, "MQTT: LedStrip [#%d] is [Off]", i);
                                iot.pub_msg(msg);
                        }
                }
        }

        else
        {
                int num_parameters = iot.inline_read(incoming_msg);
                int x = atoi(iot.inline_param[0]);

                if (strcmp(iot.inline_param[1], "on") == 0 && x < NUM_SW)
                {
                        s[x]->turnOn();
                }
                else if (strcmp(iot.inline_param[1], "off") == 0 && x < NUM_SW)
                {
                        s[x]->turnOff();
                }
        }
}
void notifyMQTT()
{
        static float lastval[] = {0.0, 0.0, 0.0};
        char msg[50];

        for (int i = 0; i < NUM_SW; i++)
        {
                if (s[i]->swState != lastval[i])
                {
                        lastval[i] = s[i]->swState;
                        if (s[i]->usePWM){
                        sprintf(msg, "Change: LedStrip [#%d] changed to [%.1f]", i, s[i]->swState);
                        }
                        else{
                                sprintf(msg, "Change: LedStrip [#%d] is now [%s]", i, s[i]->swState? "On":"Off");

                        }
                        iot.pub_msg(msg);
                }
        }
}

void setup()
{
        s0.useButton = false;
        s0.usePWM = false;
        s0.RelayON_def = true;
        s0.ButtonPressed_def = LOW;
        s0.SensorDetection_def = LOW;
        s0.start();

        s1.useButton = true;
        s1.usePWM = false;
        s1.RelayON_def = true;
        s1.ButtonPressed_def = LOW;
        s1.SensorDetection_def = LOW;
        s1.start();

        startIOTservices();
}

void loop()
{
        iot.looper();
        s0.looper();
        s1.looper();
        notifyMQTT();
        delay(100);
}
