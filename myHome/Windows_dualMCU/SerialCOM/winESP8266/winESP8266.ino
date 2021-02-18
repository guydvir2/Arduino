// #include <myIOT2.h>
#include <Arduino.h>
#include "vars.h"

// #include "myIOT_settings.h"
// #include "win_param.h"

#include <myDisplay.h>
myOLED OLED(64, 128);

// ~~~~~~~ Additional Services ~~~~~~~~
// bool services_chk()
// {
//         if (!(WiFi.isConnected() && iot.mqttClient.connected() && iot.NTP_OK))
//         {
//                 char a[50];
//                 sprintf(a, "Services error: WiFi[%s] MQTT[%s] NTP[%s]",
//                         WiFi.isConnected() ? "OK" : "FAIL", iot.mqttClient.connected() ? "OK" : "FAIL", iot.NTP_OK ? "OK" : "FAIL");
//                 iot.pub_log(a);
//                 return 0;
//         }
//         else
//         {
//                 return 1;
//         }
// }
// void check_bootclockLOG()
// {
//         char a[100];
//         char clk[20];
//         char dat[20];
//         const byte MIN_HRS_BETWEEN_RESET = 24;                                                                           /* 1 day between 2 consq. resets */
//         const byte MAX_HRS_ALL_RESETS = 24 * 7;                                                                          /* 1 week between all resets archived ( default is 3 stores ) */
//         unsigned long first_to_last = iot.get_bootclockLOG(0) / 3600 - iot.get_bootclockLOG(iot.bootlog_len - 1) / 3600; // hours
//         unsigned long first_to_second = iot.get_bootclockLOG(0) / 3600 - iot.get_bootclockLOG(1) / 3600;                 //hours
//         unsigned int rem_a = iot.get_bootclockLOG(0) % 3600 - iot.get_bootclockLOG(iot.bootlog_len - 1) % 3600;
//         unsigned int rem_b = iot.get_bootclockLOG(0) % 3600 - iot.get_bootclockLOG(1) % 3600;

//         if (first_to_last < MAX_HRS_ALL_RESETS)
//         {
//                 iot.convert_epoch2clock(first_to_last * 3600 + rem_a, 0, clk, dat);
//                 sprintf(a, "Reset Errors: got [%d] resets in [%s]. Limit is [%d]hrs", iot.bootlog_len, clk, MAX_HRS_ALL_RESETS);
//                 iot.pub_log(a);
//         }
//         if (first_to_second < MIN_HRS_BETWEEN_RESET)
//         {
//                 iot.convert_epoch2clock(first_to_second * 3600 + rem_b, 0, clk, dat);
//                 sprintf(a, "Reset Errors: got [%d] resets in [%s]. Limit is [%d]hrs", 2, clk, MIN_HRS_BETWEEN_RESET);
//                 iot.pub_log(a);
//         }
// }
// void check_reboot_reason()
// {
//         static bool checkAgain = true;
//         if (checkAgain)
//         {
//                 if (iot.mqtt_detect_reset != 2)
//                 {
//                         char a[30];
//                         checkAgain = false;
//                         if (iot.mqtt_detect_reset == 0)
//                         {
//                                 sprintf(a, "Boot Type: [%s]", "Boot");
//                         }
//                         else if (iot.mqtt_detect_reset == 1)
//                         {
//                                 sprintf(a, "Boot Type: [%s]", "Quick-Reset");
//                         }
//                         iot.pub_log(a);
//                 }
//         }
// }
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const byte pinUp = 2;
const byte pinDown = 3;
bool curstate = 0;
#define SW_PRESSED LOW
bool swUp_lastState = !SW_PRESSED;
bool swDown_lastState = !SW_PRESSED;
byte debounce_delay = 50;

void autoOff_clkUpdate()
{
        if (currentRelay_state == WIN_STOP)
        {
                autoOff_clk = 0;
        }
        else
        {
                autoOff_clk = millis();
        }
}
void autoOff_looper(int duration = autoOff_time)
{
        if (useAutoOff)
        {
                if (autoOff_clk != 0 && millis() > duration * 1000UL + autoOff_clk)
                {
                        makeSwitch(WIN_STOP);
                }
        }
}
void post_relay_change(byte &x)
{
        char a[5];
        char t[50];

        switch (x % MQTT_OFFSET)
        {
        case WIN_STOP:
                strcpy(a, "Off");
                break;
        case WIN_UP:
                strcpy(a, "Up");
                break;
        case WIN_DOWN:
                strcpy(a, "Down");
                break;
        }
        if (x < MQTT_OFFSET)
        {
                sprintf(t, "Button");
                // sprintf(t, "Button: Switch [%s]", a);
        }
        else
        {
                sprintf(t, "MQTT");
                // sprintf(t, "MQTT: Switch [%s]", a);
        }
        sprintf(a,"%d",x);
        OLED.CenterTXT("from", t,"x_val",a);
        //iot.pub_msg(t);
}
bool det_relay_change(byte &x)
{
        byte x1 = x % MQTT_OFFSET;
        if (x1 != currentRelay_state)
        {
                currentRelay_state = x1;
                if (useAutoOff)
                {
                        autoOff_clkUpdate();
                }
                post_relay_change(x);
        }
}
void makeSwitch(byte state)
{
        if (currentRelay_state != state)
        {
                Serial.write(state + MQTT_OFFSET); /* notify that originof switch is MQTT cmd*/
        }
}
void readSerial_currentstate()
{
        if (Serial.available() > 0)
        {
                byte x = Serial.read();
                char a[10];
                sprintf(a, "%d", x);
                OLED.CenterTXT("income:", a);
                delay(500);
                if (x / MQTT_OFFSET == 1)
                {
                        if (x % MQTT_OFFSET == WIN_DOWN || x % MQTT_OFFSET == WIN_UP || x % MQTT_OFFSET == WIN_STOP)
                        {
                                char a[10];
                                sprintf(a, "got2: %d", x);
                                //iot.pub_msg(a);
                                det_relay_change(x);
                        }
                }
                else if ((int)(x / QUERY_OFFSET) == 1)
                {
                        if (x % QUERY_OFFSET == WIN_DOWN)
                        {
                                //iot.pub_msg("Down");
                        }
                        else if (x % QUERY_OFFSET == WIN_UP)
                        {
                                //iot.pub_msg("UP");
                        }
                        else if (x % QUERY_OFFSET == WIN_STOP)
                        {
                                //iot.pub_msg("OFF");
                        }
                }
                else if (x == WIN_DOWN || x == WIN_UP || x == WIN_STOP)
                {
                        char a[10];
                        sprintf(a, "got3: %d", x);
                        //iot.pub_msg(a);
                        det_relay_change(x);
                }
        }
}

void readInput(int inPin, bool &lastState)
{
        bool state = digitalRead(inPin);

        if (state != lastState)
        {
                delay(debounce_delay);
                if (digitalRead(inPin) == state)
                {
                        if (state == SW_PRESSED)
                        {
                                // byte relays_state = getWin_state();
                                if (inPin == pinUp)
                                {
                                        // if (relays_state != WIN_UP)
                                        // {
                                        makeSwitch(WIN_UP);
                                        lastState = state;
                                        // }
                                }
                                else if (inPin == pinDown)
                                {
                                        // if (relays_state != WIN_DOWN)
                                        // {
                                        makeSwitch(WIN_DOWN);
                                        lastState = state;
                                        // }
                                }
                                else
                                {
                                        // if (relays_state != WIN_STOP)
                                        // {
                                        makeSwitch(WIN_STOP);
                                        lastState = !SW_PRESSED;
                                        // }
                                }
                        }
                        else
                        {
                                makeSwitch(WIN_STOP);
                                lastState = !SW_PRESSED;
                        }
                }
        }
}
void setup()
{
        // startRead_parameters();
        // startIOTservices();
        // endRead_parameters();

        // services_chk();
        // check_bootclockLOG();
        Serial.begin(9600);
        // delay(2000);
        // makeSwitch(WIN_STOP);

        pinMode(pinDown, INPUT_PULLUP);
        pinMode(pinUp, INPUT_PULLUP);
        OLED.start();
        OLED.CenterTXT("Guy", "TEST", "TESTTT");
}
void loop()
{
        //iot.looper();
        readInput(pinDown, swDown_lastState);
        readInput(pinUp, swUp_lastState);
        // autoOff_looper();
        readSerial_currentstate();
        delay(delay_loop);
}
