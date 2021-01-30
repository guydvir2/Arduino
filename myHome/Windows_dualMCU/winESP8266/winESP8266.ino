#include <myIOT2.h>
#include "vars.h"
#include "myIOT_settings.h"
#include "win_param.h"
#include <Arduino.h>

// ~~~~~~~ Additional Services ~~~~~~~~
bool services_chk()
{
        bool ntp = iot.NTP_OK;
        bool wifi = WiFi.isConnected();
        bool mqtt = iot.mqttClient.connected();

        if (!(wifi && mqtt && ntp))
        {
                char a[50];
                sprintf(a, "Services error: WiFi[%s] MQTT[%s] NTP[%s]", wifi ? "OK" : "FAIL", mqtt ? "OK" : "FAIL", ntp ? "OK" : "FAIL");
                iot.pub_log(a);
                return 0;
        }
        else
        {
                return 1;
        }
}
void check_bootclockLOG()
{
        char a[100];
        char clk[20];
        char dat[20];
        const byte MIN_HRS_BETWEEN_RESET = 24;                                                                           /* 1 day between 2 consq. resets */
        const byte MAX_HRS_ALL_RESETS = 24 * 7;                                                                          /* 1 week between all resets archived ( default is 3 stores ) */
        unsigned long first_to_last = iot.get_bootclockLOG(0) / 3600 - iot.get_bootclockLOG(iot.bootlog_len - 1) / 3600; // hours
        unsigned long first_to_second = iot.get_bootclockLOG(0) / 3600 - iot.get_bootclockLOG(1) / 3600;                 //hours
        unsigned int rem_a = iot.get_bootclockLOG(0) % 3600 - iot.get_bootclockLOG(iot.bootlog_len - 1) % 3600;
        unsigned int rem_b = iot.get_bootclockLOG(0) % 3600 - iot.get_bootclockLOG(1) % 3600;

        if (first_to_last < MAX_HRS_ALL_RESETS)
        {
                iot.convert_epoch2clock(first_to_last * 3600 + rem_a, 0, clk, dat);
                sprintf(a, "Reset Errors: got [%d] resets in [%s]. Limit is [%d]hrs", iot.bootlog_len, clk, MAX_HRS_ALL_RESETS);
                iot.pub_log(a);
        }
        if (first_to_second < MIN_HRS_BETWEEN_RESET)
        {
                iot.convert_epoch2clock(first_to_second * 3600 + rem_b, 0, clk, dat);
                sprintf(a, "Reset Errors: got [%d] resets in [%s]. Limit is [%d]hrs", 2, clk, MIN_HRS_BETWEEN_RESET);
                iot.pub_log(a);
        }
        Serial.print("max: ");
        Serial.println(first_to_last);
        Serial.print("min: ");
        Serial.println(first_to_second);
}
void check_reboot_reason()
{
        static bool checkAgain = true;
        if (checkAgain)
        {
                if (iot.mqtt_detect_reset != 2)
                {
                        char a[30];
                        checkAgain = false;
                        if (iot.mqtt_detect_reset == 0)
                        {
                                sprintf(a, "Boot Type: [%s]", "Boot");
                        }
                        else if (iot.mqtt_detect_reset == 1)
                        {
                                sprintf(a, "Boot Type: [%s]", "Quick-Reset");
                        }
                        iot.pub_log(a);
                }
        }
}

void startGPIOs()
{
        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);
        pinMode(relayDownPin, INPUT_PULLUP);
        pinMode(relayUpPin, INPUT_PULLUP);
        allOff();
}
void allOff()
{
        digitalWrite(outputUpPin, !RelayOn);
        digitalWrite(outputDownPin, !RelayOn);
        delay(delay_switch);
        Serial.println("offcMmd");
}
void post_relay_change()
{
        bool relup = digitalRead(relayUpPin);
        bool reldown = digitalRead(relayDownPin);

        if (relup != relayUP_lastState || reldown != relayDOWN_lastState) /* Change in one or more relays*/
        {
                char a[5];
                char t[50];
                byte swstate = check_current_relState();
                relayUP_lastState = relup;
                relayDOWN_lastState = reldown;

                switch (swstate)
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
                sprintf(t, "Button: Switched [%s]", a);
                iot.pub_msg(t);
        }
}
byte check_current_relState()
{
        bool relup = digitalRead(relayUpPin);
        bool reldown = digitalRead(relayDownPin);

        if (relup && reldown == !RelayOn)
        {
                return WIN_STOP;
        }
        else if (relup == RelayOn)
        {
                return WIN_UP;
        }
        else
        {
                return WIN_DOWN;
        }
}
void makeSwitch(byte state)
{
        byte restate = check_current_relState(); /* check current relays state */
        // Serial.print("relay state is: ");
        // Serial.println(restate);
        // char t[150];
        // sprintf(t, "[before]: relay_up[%d]; relay_down[%d]; out_up[%d]; out_down[%d];",
        //         digitalRead(relayUpPin), digitalRead(relayDownPin), digitalRead(outputUpPin), digitalRead(outputDownPin));
        // Serial.println(t);
        if (restate != state)
        {
                digitalWrite(outputUpPin, digitalRead(relayUpPin));
                digitalWrite(outputDownPin, digitalRead(relayDownPin));

                delay(delay_switch);
                if (state == WIN_STOP) /* Stop */
                {
                        allOff();
                }
                else if (state == WIN_UP) /* Up */
                {
                        allOff();
                        digitalWrite(outputUpPin, RelayOn);
                        Serial.println("Switch Up");
                }
                else if (state == WIN_DOWN) /* DOWN */
                {
                        allOff();
                        digitalWrite(outputDownPin, RelayOn);
                        Serial.println("Switch Down");
                }
                else
                {
                        allOff();
                        Serial.println("off due error");
                }
        }
}

void setup()
{
        // Serial.begin(115200);
        startRead_parameters();
        startGPIOs();
        startIOTservices();
        endRead_parameters();

        services_chk();
        check_bootclockLOG();
}
void loop()
{
        iot.looper();
        post_relay_change();
        // check_reboot_reason();
        delay(delay_loop);
}
