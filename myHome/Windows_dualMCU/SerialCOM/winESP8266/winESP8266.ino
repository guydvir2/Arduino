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
        // static int counter = 0;

        doc["from"] = NAME_1;
        // doc["msg_num"] = counter++;
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
        // int msg_num = _doc["msg_num"];

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
        else if (strcmp(ACT, "Error") == 0)
        {
                sprintf(outmsg, "[%s]: [%s]; from[%s]", "Error", INFO, FROM);
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
                        char aa[40];
                        sprintf(aa, "[%s]: [%s]; from[%s]", "Error", "Recv-error", NAME_1);
                        iot.pub_msg(aa);
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