#include <myIOT2.h>
#include <Arduino.h>
#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 200
#define VER "ESP8266_0.7"

#include "winStates.h"
#include "myIOT_settings.h"
#include "win_param.h"

const byte delay_loop = 10 * LOOP_DELAY;
unsigned long autoOff_clk = 0;

void sendMSG(char *msg, char *addinfo)
{
        StaticJsonDocument<JSON_SERIAL_SIZE> doc;
        static int counter = 0;

        doc["from"] = NAME_1;
        doc["msg_num"] = counter++;
        doc["act"] = msg;
        if (addinfo == NULL)
        {
                doc["info"] = "none";
        }
        else
        {
                doc["info"] = addinfo;
        }

        serializeJson(doc, Serial);
}
void autoOff_clkUpdate()
{
        if (useAutoOff)
        {
                autoOff_clk = millis();
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
                                sendMSG("off", "Auto-off");
                        }
                }
        }
}
void Serial_CB(JsonDocument &_doc)
{
        char outmsg[100];
        const char *FROM = _doc["from"];
        const char *ACT = _doc["act"];
        const char *INFO = _doc["info"];
        int msg_num = _doc["msg_num"];

        if (strcmp(ACT, "up") == 0 || strcmp(ACT, "down") == 0 || strcmp(ACT, "off") == 0)
        {
                sprintf(outmsg, "[%s]: Window [%s]", INFO, ACT);
                iot.pub_msg(outmsg);
                if (strcmp(ACT, "off") != 0)
                {
                        autoOff_clkUpdate();
                }
        }
        else if (strcmp(ACT, "query") == 0)
        {
                sprintf(outmsg, "[%s]: %s", "Query", INFO);
                iot.pub_msg(outmsg);
        }
        else if (strcmp(ACT, "Boot") == 0)
        {
                sprintf(outmsg, "[%s]: << Power On Boot >>", NAME_0);
                iot.pub_log(outmsg);
        }
        else if (strcmp(ACT, "status") == 0)
        {
                sprintf(outmsg, "[%s]: Window [%s]", "Status", INFO);
                iot.pub_msg(outmsg);
        }
}
void readSerial()
{
        if (Serial.available())
        {
                StaticJsonDocument<JSON_SERIAL_SIZE> doc;
                DeserializationError error = deserializeJson(doc, Serial);

                if (!error)
                {
                        Serial_CB(doc);
                }
                else
                {
                        iot.pub_msg("Serial error");
                }
        }
}
void setup()
{
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
        services_chk();

        Serial.begin(9600); /* Serial is defined not using IOT - else it spits all debug msgs */
}
void loop()
{
        iot.looper();
        autoOff_looper();
        readSerial();
        delay(delay_loop);
}

// ~~~~~~~~~~~~~~~~ LCD ~~~~~~~~~~~~~~
// #include <myDisplay.h>

// myLCD lcd(2);
// const byte ROWS = 2;
// char lcd_rows[ROWS][20];
// void init_rows()
// {
//         for (int i = 0; i < ROWS; i++)
//         {
//                 strcpy(lcd_rows[i], "empty");
//         }
// }
// void cov2clk(char *time_str)
// {
//         byte days = 0;
//         byte hours = 0;
//         byte minutes = 0;
//         byte seconds = 0;

//         int sec2minutes = 60;
//         int sec2hours = (sec2minutes * 60);
//         int sec2days = (sec2hours * 24);
//         int sec2years = (sec2days * 365);

//         long unsigned time_delta = (int)(millis() / 1000);

//         days = (int)(time_delta / sec2days);
//         hours = (int)((time_delta - days * sec2days) / sec2hours);
//         minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
//         seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

//         sprintf(time_str, "%01dd %02d:%02d:%02d", days, hours, minutes, seconds);
// }
// void post_lcd()
// {
//         lcd.clear();
//         lcd.CenterTXT(lcd_rows[0], lcd_rows[1], lcd_rows[2], lcd_rows[3]);
// }
// void update_lcd(char *msg)
// {
//         static byte c = 0;
//         for (int i = ROWS - 1; i > 0; i--)
//         {
//                 strcpy(lcd_rows[i], lcd_rows[i - 1]);
//         }
//         char retclk[14];

//         cov2clk(retclk);
//         sprintf(lcd_rows[0], "m%d:%s", c, msg);
//         // sprintf(lcd_rows[0], "%s m%d:%s", retclk, c, msg);
//         post_lcd();
//         c++;
// }
// ~~~