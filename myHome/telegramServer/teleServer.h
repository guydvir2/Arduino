#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

extern void sendFormatted_msg_tele(const char *from, const char *subj, const char *msg);

void startTelegram()
{
    client.setInsecure();
}
void sendMsg(char *msg)
{
    bot.sendMessage(CHAT_ID, msg, "");
}
void sendMsg(String &msg)
{
    bot.sendMessage(CHAT_ID, msg, "");
}

void getTelecmd(String &inmsg, String &from, String &id)
{
    char t[200];
    const char *command_set[] = {"/alarm_off", "/alarm_arm", "/boiler_on", "/boiler_off", "/wind_up", "/wind_down", "/help", "/status"};
    byte num_commands = sizeof(command_set) / sizeof(command_set[0]);
    if (inmsg == command_set[0])
    {
        iot.pub_noTopic("disarmed", "myHome/alarmMonitor");
        strcpy(t,"Alarm : Off");
    }
    else if (inmsg == command_set[1])
    {
        iot.pub_noTopic("armed_away", "myHome/alarmMonitor");
        strcpy(t,"Alarm : Armed away");
    }
    else if(inmsg == command_set[2])
    {
        iot.pub_noTopic("timeout, 90", "myHome/WaterBoiler");
        strcpy(t,"Boiler: Timeout 1:30:00");
    }
    else if (inmsg == command_set[3])
    {
        iot.pub_noTopic("off", "myHome/WaterBoiler");
        strcpy(t,"Boiler: Off");
    }
    else if(inmsg == command_set[4])
    {
        iot.pub_noTopic("up", "myHome/Windows/saloonExit");
        strcpy(t,"salootExit Windows: UP");
    }
    else if (inmsg == command_set[5])
    {
        iot.pub_noTopic("down", "myHome/Windows/saloonExit");
        strcpy(t,"salootExit Windows: DOWN");
    }
    else if(inmsg == command_set[6])
    {
        strcpy(t, "");
        for (int i = 0; i < num_commands; i++)
        {
            char a[20];
            sprintf(a, "\n%s", command_set[i]);
            strcat(t, a);
        }
    }
    else if (inmsg == command_set[7])
    {
        iot.get_timeStamp();
        sprintf(t, "status: On from %s", iot.timeStamp);
    }
    sendFormatted_msg_tele(DEV_TOPIC, "Commands", t);
}
void handleNewMessages(int numNewMessages)
{
    char sendmsg[500];

    for (int i = 0; i < numNewMessages; i++)
    {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        if (from_name == "")
        {
            from_name = "Guest";
        }
        getTelecmd(text, from_name, chat_id);
        // _ext_func(text, from_name, chat_id, sendmsg);

        // if (strcmp(sendmsg, "") != 0)
        // {
        //     bot.sendMessage(chat_id, sendmsg, "");
        // }
        Serial.println(text);
    }
}
void check_telegramServer()
{
    static unsigned long _Bot_lasttime = 0;
    const int _Bot_mtbs = 2000;

    if (millis() > _Bot_lasttime + _Bot_mtbs)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages)
        {
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        _Bot_lasttime = millis();
    }
}
