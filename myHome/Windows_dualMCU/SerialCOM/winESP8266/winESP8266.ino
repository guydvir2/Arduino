#include <myIOT2.h>
#include <Arduino.h>
#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 200
#define VER "ESP8266_0.5"

#include "winStates.h"
#include "myIOT_settings.h"
#include "win_param.h"

const char *winstate[] = {"Off", "Up", "Down"};
const byte delay_loop = 10 * LOOP_DELAY;
byte currentRelay_state = WIN_STOP;
unsigned long autoOff_clk = 0;

void autoOff_clkUpdate()
{
        if (useAutoOff)
        {
                // if (getWin_state() && currentRelay_state == WIN_STOP)
                if (currentRelay_state == WIN_STOP)
                {
                        autoOff_clk = 0;
                }
                else
                {
                        autoOff_clk = millis();
                }
        }
}
void autoOff_looper(int duration = autoOff_time)
{
        if (useAutoOff)
        {
                if (autoOff_clk != 0)
                {
                        if (millis() > duration * 1000UL + autoOff_clk)
                        {
                                autoOff_clk = 0;
                                makeSwitch(WIN_STOP);
                        }
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
        // byte x1 = x % MQTT_OFFSET;
        currentRelay_state = x % MQTT_OFFSET;
        autoOff_clkUpdate();
        post_relay_change(x);
}
void makeSwitch(byte state)
{
        // if (getWin_state() && currentRelay_state != state)
        if (currentRelay_state != state)

        {
                Serial.write(state + MQTT_OFFSET); /* notify that originof switch is MQTT cmd*/
                char e[10];
                sprintf(e, "sent: %d", state + MQTT_OFFSET);
                iot.pub_msg(e);
        }
}
// bool getWin_state()
// {
//         currentRelay_state = QUERY_INIT;
//         const byte ret = 20;
//         byte counter = 0;

//         Serial.write(QUERY);
//         while (currentRelay_state == QUERY_INIT && counter < ret)
//         {
//                 if (Serial.available())
//                 {
//                         byte x = Serial.read();
//                         currentRelay_state = x % QUERY_OFFSET;
//                 }
//                 else if (counter == ret / 2)
//                 {
//                         Serial.write(QUERY);
//                 }
//                 delay(10);
//                 counter++;
//         }
//         if (currentRelay_state != QUERY_INIT)
//         {
//                 char b[10];
//                 sprintf(b, "OK #%d: %d", counter, currentRelay_state);
//                 iot.pub_msg(b);
//                 return 1;
//         }
//         else
//         {
//                 return 0;
//         }
// }
void Serial_callbacks(byte &x)
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
        // else
        // {
        //         iot.pub_log("OTHER");
        // }
}
void readSerial()
{
        if (Serial.available())
        {
                byte x = Serial.read();
                char r[12];
                sprintf(r, "READ: %d", x);
                iot.pub_msg(r);
                Serial_callbacks(x);
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
        readSerial();
        delay(delay_loop);
}
