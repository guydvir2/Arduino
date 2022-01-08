#include <Arduino.h>
#include <myIOT2.h>
#include "telegram_param.h"

// ********** Sketch Services  ***********
#define VER "telegramSever_v0.1"

myIOT2 iot;

void startIOTservices()
{
        iot.useSerial = paramJSON["useSerial"];
        iot.useWDT = paramJSON["useWDT"];
        iot.useOTA = paramJSON["useOTA"];
        iot.useResetKeeper = paramJSON["useResetKeeper"];
        iot.useDebug = paramJSON["useDebugLog"];
        iot.debug_level = paramJSON["debug_level"];
        iot.useNetworkReset = paramJSON["useNetworkReset"];
        iot.useextTopic = paramJSON["useextTopic"];
        iot.noNetwork_reset = paramJSON["noNetwork_reset"];
        strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
        strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
        strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
        // strcpy(iot.extTopic, paramJSON["extTopic"]);

        iot.start_services(addiotnalMQTT);
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                sprintf(msg, "Status: OK");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
                sprintf(msg, "Help: Commands #3 - [NEW]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                sprintf(msg, "Ver: Ver:%s", VER);
                iot.pub_msg(msg);
        }
}

MQTT_msg incoming_mqtt;
char *Telegram_Nick = "TelegramServer";
byte time_check_messages = 10; //sec

myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
        String command_set[] = {"whois_online", "status", "reset", "help", "whoami"};
        byte num_commands = sizeof(command_set) / sizeof(command_set[0]);
        String comp_command[num_commands];
        char prefix[100], prefix2[100];
        char t1[50], t2[50];

        sprintf(snd_msg, ""); // when not meeting any conditions, has to be empty

        from.toCharArray(t1, from.length() + 1);
        in_msg.toCharArray(t2, in_msg.length() + 1);

        sprintf(prefix, "/%s_", Telegram_Nick);
        sprintf(prefix2, "from user: %s\ndevice replies: %s\ncommand: %s\n~~~~~~~~~~~~~~~~~~~~\n ", t1, Telegram_Nick, t2);

        for (int i = 0; i < num_commands; i++)
        {
                comp_command[i] = prefix;
                comp_command[i] += command_set[i];
        }

        if (in_msg == "/whois_online")
        {
                sprintf(snd_msg, "%s%s", prefix2, Telegram_Nick);
        }
        else if (in_msg == comp_command[1])
        {
                // giveStatus(t1);
                sprintf(snd_msg, "%s%s", prefix2, t1);
        } // status
        else if (in_msg == comp_command[2])
        {
                sprintf(snd_msg, "%s", prefix2);
                iot.sendReset("Telegram");
        } // reset
        else if (in_msg == comp_command[3])
        {
                char t[50];
                sprintf(snd_msg, "%sCommands Available:\n", prefix2, Telegram_Nick);
                for (int i = 0; i < num_commands; i++)
                {
                        command_set[i].toCharArray(t, 30);
                        sprintf(t1, "%s\n", t);
                        strcat(snd_msg, t1);
                }

        } // all_commands
        else if (in_msg == comp_command[4])
        {
                sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, "DEVICE_TOPIC");
        } // whoami
}
bool chekcTelegram_topic(char *topic, MQTT_msg &msg)
{
        if (strcmp(iot.mqqt_ext_buffer[0], topic) == 0)
        {
                sprintf(msg.from_topic, "%s", iot.mqqt_ext_buffer[0]);
                sprintf(msg.msg, "%s", iot.mqqt_ext_buffer[1]);
                sprintf(msg.device_topic, "%s", iot.mqqt_ext_buffer[2]);
                for (int i = 0; i < 3; i++)
                {
                        Serial.println(iot.mqqt_ext_buffer[i]);
                }
                return 1;
        }
        else
        {
                return 0;
        }
}
void listenMQTT_forTelegram()
{
        if (chekcTelegram_topic(iot.extTopic, incoming_mqtt))
        {
                teleNotify.send_msg2(String(incoming_mqtt.msg));
                for (int i = 0; i < 3; i++)
                {
                        sprintf(iot.mqqt_ext_buffer[i], "%s", "");
                }
        }
}

int PIRpin = 2;
bool lastSenseState = false;



void setup()
{
        startRead_parameters();
        startIOTservices();
        endRead_parameters();

        teleNotify.begin(telecmds);
        iot.pub_ext("Boot");
        #if USE_EMAIL
        sendEmail("Hi- i'm fine", "guydvir2@gmail.com", "Testing123");
        #endif
}
void loop()

{
        iot.looper();
        teleNotify.looper();
        listenMQTT_forTelegram();
        delay(100);
}
