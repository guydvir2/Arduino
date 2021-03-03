#include <myIOT2.h>
#include <Arduino.h>
#include "vars.h"
#include "myIOT_settings.h"
#include "win_param.h"

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
        char t[50];
        sprintf(t, "%s: Switch [%s]", x < MQTT_OFFSET ? "Button" : "MQTT", winstate[x % MQTT_OFFSET]);
        iot.pub_msg(t);
}
bool update_relayState(byte &x)
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
void Serial_cmd_callbacks(byte &x)
{
        if ((x <= WIN_DOWN && x >= WIN_STOP) || (x <= MQTT_OFFSET + WIN_DOWN && x >= MQTT_OFFSET + WIN_STOP))
        {
                update_relayState(x);
        }
        else if (x <= WIN_DOWN + QUERY_OFFSET && x >= WIN_STOP + QUERY_OFFSET)
        {
                char a[8];
                sprintf(a, "Query: Window [%s]", winstate[x % QUERY_OFFSET]);
                iot.pub_msg(a);
        }
        else if (x == BOOT) /* Incoming Query Result answered by Arduino */
        {
                iot.pub_log("Reset: Arduino board");
        }
}
void get_cmd_Serial()
{
        if (Serial.available() > 0)
        {
                byte x = Serial.read();
                Serial_cmd_callbacks(x);        
        }
}

void setup()
{
        startRead_parameters();
        startIOTservices();
        endRead_parameters();

        services_chk();
        // check_bootclockLOG();
        Serial.begin(9600); /* Serial is defined not using IOT - else it spits all debug msgs */
}
void loop()
{
        iot.looper();
        autoOff_looper();
        get_cmd_Serial();
        delay(delay_loop);
}
