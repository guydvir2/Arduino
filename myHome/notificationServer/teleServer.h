#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <myLOG.h>

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
#define MAX_ALLOWED_USERS 10
flashLOG Users;

// char users[MAX_ALLOWED_USERS][10];
const String secret_password = "/dvirs";
extern void formatted_SMS(const char *from, const char *subj, const char *msg, const char *time, char Msg[]);
// void start_users()
// {
//     const char *a = "A";
//     for (int i = 0; i < MAX_ALLOWED_USERS; i++)
//     {
//         sprintf(users[i], "empty");
//     }
// }
void startTelegram()
{
    client.setInsecure();
}
bool send_SMS(char *msg)
{
    if (bot.sendMessage(CHAT_ID, msg, ""))
    {
        Serial.println("Telegram Message sent OK");
        return 1;
    }
    else
    {
        return 0;
    }
}
bool send_SMS(String &msg)
{
    if (bot.sendMessage(CHAT_ID, msg, ""))
    {
        Serial.println("Telegram Message sent OK");
        return 1;
    }
    else
    {
        return 0;
    }
}

void getTelecmd(String &inmsg, String &from, String &id)
{
    char t[200];
    const char *command_set[] = {"/alarm_off", "/alarm_arm", "/boiler_on", "/boiler_off", "/wind_up", "/wind_down",
                                 "/all_wind_down", "/all_wind_down_arm", "/wind_up_disarm", "/help", "/status"};
    byte num_commands = sizeof(command_set) / sizeof(command_set[0]);
    if (inmsg == command_set[0])
    {
        iot.pub_noTopic("disarmed", "myHome/alarmMonitor");
        strcpy(t, "Alarm : Off");
    }
    else if (inmsg == command_set[1])
    {
        iot.pub_noTopic("armed_away", "myHome/alarmMonitor");
        strcpy(t, "Alarm : Armed away");
    }
    else if (inmsg == command_set[2])
    {
        iot.pub_noTopic("timeout, 90", "myHome/WaterBoiler");
        strcpy(t, "Boiler: Timeout 1:30:00");
    }
    else if (inmsg == command_set[3])
    {
        iot.pub_noTopic("off", "myHome/WaterBoiler");
        strcpy(t, "Boiler: Off");
    }
    else if (inmsg == command_set[4])
    {
        iot.pub_noTopic("up", "myHome/Windows/saloonExit");
        strcpy(t, "salootExit Windows: UP");
    }
    else if (inmsg == command_set[5])
    {
        iot.pub_noTopic("down", "myHome/Windows/saloonExit");
        strcpy(t, "salootExit Windows: DOWN");
    }
    else if (inmsg == command_set[6])
    {
        iot.pub_noTopic("down", "myHome/Windows");
        strcpy(t, "All Windows: DOWN");
    }
    else if (inmsg == command_set[7])
    {
        iot.pub_noTopic("down", "myHome/Windows");
        iot.pub_noTopic("armed_away", "myHome/alarmMonitor");

        strcpy(t, "All Windows: DOWN; Alarm: Armed away");
    }
    else if (inmsg == command_set[8])
    {
        iot.pub_noTopic("up", "myHome/Windows");
        iot.pub_noTopic("disarmed", "myHome/alarmMonitor");

        strcpy(t, "salootExit Windows: UP; Alarm: Disarmed");
    }
    else if (inmsg == command_set[9])
    {
        strcpy(t, "");
        for (int i = 0; i < num_commands; i++)
        {
            char a[20];
            sprintf(a, "\n\n%s", command_set[i]);
            strcat(t, a);
        }
    }
    else if (inmsg == command_set[10])
    {
        iot.get_timeStamp();
        sprintf(t, "status: On from %s", iot.timeStamp);
    }
    char Msg[300];
    iot.get_timeStamp();
    formatted_SMS(from.c_str(), "Commands", t, iot.timeStamp, Msg);
    send_SMS(Msg);
}
bool check_user(String &new_id)
{
    int x = 0;
    //Users.getnumlines();
    char temp[15];
    Serial.print("total records: ");
    Serial.println(x);
    // for (int i = 0; i < x; i++)
    // {
    //     if (Users.readline(i, temp))
    //     {
    //         Serial.print("got user: ");
    //         Serial.println(temp);
    //     }
    //     else
    //     {
    //         Serial.println("err reading");
    //     }
    //     if (new_id == temp)
    //     {
    //         Serial.println("user match");
    //         return true;
    //     }
    // }
    return false;
}
void handleNewMessages(int numNewMessages)
{
    char sens_SMS[500];

    for (int i = 0; i < numNewMessages; i++)
    {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;

        if (from_name == "")
        {
            from_name = "Guest";
        }
        // if (check_user(chat_id))
        // {
        //     Serial.println("user exists in records");
        // }
        // else
        // {
        //     if (text == secret_password)
        //     {
        //         char t[20];
        //         strcpy(t, chat_id.c_str());
        //         Users.write(t);
        //         // Users.writeNow();
        //         Serial.println("user added");
        //     }
        //     else{
        //     Serial.println("no such user");
        //     }
        //     // sprintf(users[0], "%s", chat_id.c_str());
        //     // sprintf(sens_SMS, "Hello %s\nYou need to enter secret password to continue.", from_name);
        // }
        getTelecmd(text, from_name, chat_id);

        // if (strcmp(sens_SMS, "") != 0)
        // {
        //     bot.sendMessage(chat_id, sens_SMS, "");
        // }
        // Serial.println(text);
        // Serial.println(chat_id);
    }
}
void check_telegramServer()
{
    static unsigned long _Bot_lasttime = 0;
    const int _Bot_mtbs = 1000;

    if (millis() > _Bot_lasttime + _Bot_mtbs)
    {
        unsigned long lasttime = millis();
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages)
        {
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        _Bot_lasttime = millis();
        Serial.print("time:");
        Serial.println((_Bot_lasttime - lasttime));
    }
}

void formatted_SMS(const char *from, const char *subj, const char *msg, const char *time, char Msg[])
{
    iot.get_timeStamp();
    sprintf(Msg, ">> From: @%s\n>> Subject: %s\n>> Message: %s\n>> Time: %s \n\n~~ sent: @%s %s", from, subj, msg, time, DEV_TOPIC, mserver_ver);
}
