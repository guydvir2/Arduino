#include <myIOT2.h>
#include <EMailSender.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

void sendEmail(char *outMsg, char *MsgSubj, char *sendTo)
{
        EMailSender emailSend("guydvir.tech@gmail.com", "GdSd13100301", "guyDvir");

        EMailSender::EMailMessage message;
        message.subject = MsgSubj;
        message.message = outMsg;
        EMailSender::Response resp = emailSend.send(sendTo, message);
        Serial.println("Sending status: ");

        Serial.println(resp.status);
        Serial.println(resp.code);
        Serial.println(resp.desc);
}

void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();

#elif USE_SIMPLE_IOT == 0
        // read_flashParameter();
        // Serial.begin(115200);
        startIOTservices();
#endif
        sendEmail("MESSSSSAGE", "THIS IS SUBJECT", "guydvir2@gmail.com");
}

void loop()
{
        iot.looper();
        // delay(100);
        // extTopic_looper();
}
