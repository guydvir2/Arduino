#include <Arduino.h>
#include <myIOT2.h>
#include <secretsIOT8266.h>
#include "myIOT_settings.h"
#include "teleServer.h"

const char *mserver_ver = "telegram_Server_v0.1";

// ~~~~~~~~~~~~~~ Listening to sms Topic ~~~~~~~~~~~~~~
void convMQTT_toJSON(char *inmsg, JsonDocument &DOC)
{
    deserializeJson(DOC, (const char *)inmsg);
    const char *from = DOC["from"];
    const char *subj = DOC["sub"];
    const char *msg = DOC["body"];
}
bool check_Tele_MQTTtopic()
{
    if (strcmp(iot.extTopic_msg.from_topic, TELE_MQTT_TOPIC) == 0)
    {
        DynamicJsonDocument DOC(400);
        convMQTT_toJSON(iot.extTopic_msg.msg, DOC);
        sendFormatted_msg_tele(DOC["from"], DOC["sub"], DOC["body"]);
        iot.clear_ExtTopicbuff();
        return 1;
    }
    else
    {
        return 0;
    }
}
void sendMSGtoMQTT(const char *from, const char *subj, const char *msg)
{
    String sms = "{\"from\":\"" + String(from) + "\",\"body\":\"" + String(msg) + "\",\"sub\":\"" + String(subj) + "\"}";
    iot.pub_sms(sms);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void sendFormatted_msg_tele(const char *from, const char *subj, const char *msg)
{
    char Msg[300];
    iot.get_timeStamp();
    sprintf(Msg, ">> Message from: @%s\n>>Subjectm: %s\n>> Message: %s\n\n~~ sent: @%s %s", from, subj, msg, DEV_TOPIC, iot.timeStamp);
    sendMsg(Msg);
}

void setup()
{
    startIOTservices();
    startTelegram();
    sendMSGtoMQTT(DEV_TOPIC, "info", "Server boot-up");
}
void loop()
{
    iot.looper();
    check_Tele_MQTTtopic(); /* intercept MQTT messages to post on Telegram*/
    check_telegramServer(); /* loop Telegram Server */
    delay(100);
}
