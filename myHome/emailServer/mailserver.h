#include <EMailSender.h>
#include <ArduinoJson.h>

#define SEND_TO "guy.ipaq@gmail.com"
#define GMAIL_USER "guydvir.tech@gmail.com"
#define GMAIL_P "GdSd13100301"
#define GMAIL_FROM "myHome"

EMailSender emailSend(GMAIL_USER, GMAIL_P, GMAIL_USER, GMAIL_FROM);

void sendEmail(const char *subj, const char *outmsg, char *sendto = SEND_TO)
{
    EMailSender::EMailMessage message;
    message.subject = subj;
    message.message = outmsg;

    EMailSender::Response resp = emailSend.send(sendto, message);

    Serial.print("Sending status: ");
    Serial.println(resp.status);
    // Serial.println(resp.code);
    // Serial.println(resp.desc);
}
void JSON2sendEmail(char *inmsg)
{
    DynamicJsonDocument email(1000);
    deserializeJson(email, (const char *)inmsg);
    const char *from = email["from"];
    const char *subj = email["sub"];
    const char *msg = email["body"];
    // email["time"] = clk;
    sendEmail(subj, msg);
}
void convMQTT_toJSON(char *inmsg, JsonDocument &DOC)
{
    deserializeJson(DOC, (const char *)inmsg);
    const char *from = DOC["from"];
    const char *subj = DOC["sub"];
    const char *msg = DOC["body"];
}
