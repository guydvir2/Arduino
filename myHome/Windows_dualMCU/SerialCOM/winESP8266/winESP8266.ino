#include <myIOT2.h>
#include "vars.h"
#include "myIOT_settings.h"
#include "win_param.h"
#include <Arduino.h>

// ~~~~~~~ Additional Services ~~~~~~~~
bool services_chk()
{
        if (!(WiFi.isConnected() && iot.mqttClient.connected() && iot.NTP_OK))
        {
                char a[50];
                sprintf(a, "Services error: WiFi[%s] MQTT[%s] NTP[%s]",
                        WiFi.isConnected() ? "OK" : "FAIL", iot.mqttClient.connected() ? "OK" : "FAIL", iot.NTP_OK ? "OK" : "FAIL");
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
        // Serial.print("max: ");
        // Serial.println(first_to_last);
        // Serial.print("min: ");
        // Serial.println(first_to_second);
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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
void post_relay_change(int &x)
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
                sprintf(t, "Button: Switch [%s]", a);
        }
        else
        {
                sprintf(t, "MQTT: Switch [%s]", a);
        }
        iot.pub_msg(t);
}
bool det_relay_change(int &x)
{
        int x1 = x % MQTT_OFFSET;
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
                Serial.write(state + MQTT_OFFSET);
        }
}
void readSerial_currentstate()
{
        if (Serial.available() > 0)
        {
                int x = Serial.read();
                if (x % MQTT_OFFSET == WIN_DOWN || x % MQTT_OFFSET == WIN_UP || x % MQTT_OFFSET == WIN_STOP)
                {
                        det_relay_change(x);
                }
        }
}
void setup()
{
        Serial.begin(115200);

        startRead_parameters();
        startIOTservices();
        endRead_parameters();

        services_chk();
        check_bootclockLOG();
}
void loop()
{
        iot.looper();
        autoOff_looper();
        readSerial_currentstate();
        delay(delay_loop);
}
