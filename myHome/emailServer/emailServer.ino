#include <Arduino.h>
#include <myIOT2.h>
#include "myIOT_settings.h"
#include "mailserver.h"

const char *mserver_ver = "mailServer_v0.1";

bool email_looper()
{
    if (strcmp(iot.extTopic_msg.from_topic, MAIL_MQTT_TOPIC) == 0)
    {
        DynamicJsonDocument DOC(1000);
        convMQTT_toJSON(iot.extTopic_msg.msg, DOC);
        sendEmail(DOC["sub"], DOC["body"]);
        iot.clear_ExtTopicbuff();
        return 1;
    }
    else
    {
        return 0;
    }
}
void construct_2email(const char *from, const char *subj, const char *msg)
{
    StaticJsonDocument<1000> DOC;
    iot.get_timeStamp();
    String s = String(DEV_TOPIC) + " send message @" + String(from);
    String m = ">> Time: " + String(iot.timeStamp) + "<br>" +
               ">> iot Device: " + String(from) + "<br>" +
               ">> Message: " + String(msg) + "<br>" +
               ">> sent using: " + String(mserver_ver);

    String mail = "{\"from\":\"" + String(from) +
                  String("\",\"body\":\"") + m +
                  String("\",\"sub\":\"") + s +
                  String("\"}");
    deserializeJson(DOC, mail);
    iot.pub_email(DOC);
}
void setup()
{
    startIOTservices();
    construct_2email(DEV_TOPIC, "BootUp notification", "We're UP");
}
void loop()
{
    iot.looper();
    email_looper();
    delay(100);
}
