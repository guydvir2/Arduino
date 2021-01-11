#include <myIOT2.h>

#define DEV_TOPIC "notifServer"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"
#define NOTIF_MQTT_TOPIC PREFIX_TOPIC "/sms"
#define SEND_TO "guy.ipaq@gmail.com"

myIOT2 iot;
extern const char *mserver_ver;
extern const char *EMAIL_PREFIX;
extern const char *SMS_PREFIX;
extern bool send_SMS(char *msg);
extern bool send_SMS(String *msg);
extern void formatted_email(const char *from, const char *subj, const char *msg, const char *time, JsonDocument &DOC);
extern void formatted_SMS(const char *from, const char *subj, const char *msg, const char *time, char Msg[]);
extern bool sendEmail(const char *subj, const char *outmsg, const char *sendto = SEND_TO);

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "Status: ");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", mserver_ver, iot.ver, isESP8266 ? "ESP8266" : "ESP32");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [email,[<msg>,<subject>,<from>]]");
        iot.pub_msg(msg);
    }
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);

        if (strcmp(iot.inline_param[0], EMAIL_PREFIX) == 0)
        {
            DynamicJsonDocument DOC(500);
            iot.get_timeStamp();
            formatted_email(iot.inline_param[3], // from
                            iot.inline_param[2], // subj
                            iot.inline_param[1], // msg
                            iot.timeStamp,       // time  <--- not entered by user
                            DOC);
            sendEmail(DOC["sub"], DOC["body"]);
        }
        else if (strcmp(iot.inline_param[0], SMS_PREFIX) == 0)
        {
            char Msg[200];
            iot.get_timeStamp();
            formatted_SMS(iot.inline_param[3], // from
                           iot.inline_param[2], // subj
                           iot.inline_param[1], // msg
                           iot.timeStamp,       // time  <--- not entered by user
                           Msg);
            send_SMS(Msg);
        }
        for (int i = 0; i < iot.num_param; i++)
        {
            strcpy(iot.inline_param[i], "");
        }
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = true;
    iot.resetFailNTP = true;
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);
    iot.extTopic[0] = NOTIF_MQTT_TOPIC;

    iot.start_services(addiotnalMQTT);
}
