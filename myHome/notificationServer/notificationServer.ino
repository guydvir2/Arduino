#include <Arduino.h>
#include <myIOT2.h>
#include "myIOT_settings.h"

/* Select Services */
#define USE_EMAIL_NOTIF 1
#define USE_SMS_NOTIF 1
/* ~~~~~~~~~~~~~~~ */

#define SMS_PREFIX "sms"
#define EMAIL_PREFIX "email"
#define LED_ON HIGH
#define LED_OFF !LED_ON

#if USE_EMAIL_NOTIF
#include "mailserver.h"
#endif

#if USE_SMS_NOTIF
#include "teleServer.h"
#endif

const byte ledPin = 2;
const char *mserver_ver = "nServer_v0.2";

/* Reading MQTT Topic for new Messages*/
void incomeMQTT_2JSON(char *inmsg, JsonDocument &DOC)
{
    deserializeJson(DOC, (const char *)inmsg);
    const char *from = DOC["from"] | "none";
    const char *subj = DOC["sub"] | "none";
    const char *msg = DOC["body"] | "none";
    const char *type = DOC["type"] | "email";
    const char *time = DOC["time"] | "none";
}
bool extTopics_looper()
{
    if (strcmp(iot.extTopic_msg.msg, "") != 0)
    {
        DynamicJsonDocument DOC(500);
        if (strcmp(iot.extTopic_msg.from_topic, NOTIF_MQTT_TOPIC) == 0)
        {
            incomeMQTT_2JSON(iot.extTopic_msg.msg, DOC);
            if (strcmp(DOC["type"], EMAIL_PREFIX) == 0)
            {
#if USE_EMAIL_NOTIF
                formatted_email(DOC["from"], DOC["sub"], DOC["body"], DOC["time"], DOC);
                if (sendEmail(DOC["sub"], DOC["body"]))
                {
                    Serial.println("Email sent OK");
                    iot.pub_log("[eMail]: sent successfully");
                    iot.clear_ExtTopicbuff();
                    return 1;
                }
#endif
            }
            else if (strcmp(DOC["type"], SMS_PREFIX) == 0)
            {
                char Msg[300];
#if USE_SMS_NOTIF
                formatted_SMS(DOC["from"], DOC["sub"], DOC["body"], DOC["time"], Msg);
                if (send_SMS(Msg))
                {
                    iot.pub_log("[SMS]: sent successfully");
                    iot.clear_ExtTopicbuff();

                    return 1;
                }
#endif
            }
        }
        return 0;
    }
    else
    {
        return 0;
    }
}

/* Constructing a JSON fromat and sending to MQTT Topic */
void pub_MSG(const char *type, const char *from, const char *subj, const char *msg, const char *time)
{
    String inmsg = "{\"type\":\"" + String(type) +
                   "\",\"from\":\"" + String(from) +
                   "\",\"body\":\"" + String(msg) +
                   "\",\"sub\":\"" + String(subj) +
                   "\",\"time\":\"" + String(time) + "\"}";

    iot.pub_sms(inmsg);
}

/* Blinking Led*/
void startIO()
{
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LED_ON);
}
void liveBlink(int X = 5000, byte y = 3)
{
    static unsigned long lastBlink_clk = 0;

    if (millis() - lastBlink_clk > X)
    {
        byte i = 0;
        while (i < y)
        {
            digitalWrite(ledPin, LED_OFF);
            delay(20);
            digitalWrite(ledPin, LED_ON);
            delay(20);
            i++;
        }
        lastBlink_clk = millis();
    }
}

void setup()
{
    startIO();
    startIOTservices();
#if USE_SMS_NOTIF
    startTelegram();
#endif
    iot.get_timeStamp();
    pub_MSG(SMS_PREFIX, DEV_TOPIC, "בדיקה", "Boot!", iot.timeStamp);
    pub_MSG(EMAIL_PREFIX, "FROM", "SUBJECT", "MSG", iot.timeStamp);
}
void loop()
{
    iot.looper();
    extTopics_looper();
#if USE_SMS_NOTIF
    check_telegramServer();
#endif
    liveBlink();
    delay(100);
}
