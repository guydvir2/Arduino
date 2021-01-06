#include <EMailSender.h>
#include <ArduinoJson.h>

#define SEND_TO "guy.ipaq@gmail.com"
#define GMAIL_FROM "myHome"

EMailSender emailSend(GMAIL_USER, GMAIL_P, GMAIL_USER, GMAIL_FROM);

bool sendEmail(const char *subj, const char *outmsg, const char *sendto = SEND_TO)
{
    EMailSender::EMailMessage message;
    message.subject = subj;
    message.message = outmsg;

    EMailSender::Response resp = emailSend.send(sendto, message);

    Serial.print("Sending status: ");
    Serial.println(resp.status);
    return resp.status;
    // Serial.println(resp.code);
    // Serial.println(resp.desc);
}
void formatted_email(const char *from, const char *subj, const char *msg, const char *time, JsonDocument &DOC)
{
    String s = "@" + String(from);
    String m = ">> From: " + String(from) + "<br>" +
               ">> Subject: " + String(subj) + "<br>" +
               ">> Message: " + String(msg) + "<br>" +
               ">> Time: " + String(time) + "<br>" + "<br>" +
               "~~ sent: @" + DEV_TOPIC + " " + String(mserver_ver);

    String mail = "{\"from\":\"" + String(from) +
                  String("\",\"body\":\"") + m +
                  String("\",\"sub\":\"") + s +
                  String("\"}");
    deserializeJson(DOC, mail);
}
