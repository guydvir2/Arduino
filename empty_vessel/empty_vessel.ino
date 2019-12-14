#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.1"
#define USE_NOTIFY_TELE false

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true
#define USE_WDT false
#define USE_OTA true
#define USE_RESETKEEPER true
#define USE_FAILNTP true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "solarPanel"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "TESTS"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

float Vbat         = 0;
float Vbat_precent = 0;
float Vbat_max     = 4.2;
float Vbat_min_to_operate = 3.0;
long LowBat_sleepPeriod   = 1000000*10L; // in seconds
long sleepPeriod_1 = 1000000*60*10L;
long lastsleep =0;

    // ~~~~~~~ Using Telegram ~~~~~~~~~~~~~~
    char *Telegram_Nick = DEVICE_TOPIC;
int time_check_messages = 2; //sec
#if USE_NOTIFY_TELE
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
        String command_set[] = {"whois_online", "status", "reset", "whoami", "help", "up", "down", "off"};
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
                sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, DEVICE_TOPIC);
        } // whoami
        else if (in_msg == comp_command[4])
        {
                char t[50];
                sprintf(snd_msg, "%sCommands Available:\n", prefix2, Telegram_Nick);
                for (int i = 0; i < num_commands; i++)
                {
                        command_set[i].toCharArray(t, 30);
                        sprintf(t1, "%s\n", t);
                        strcat(snd_msg, t1);
                }
        }
        //        else if (in_msg==comp_command[5]) {
        //                switchIt("Telegram", "up");
        //                sprintf(snd_msg,"%s[UP]",prefix2 );
        //        }//up
        //        else if (in_msg==comp_command[6]) {
        //                switchIt("Telegram", "down");
        //                sprintf(snd_msg,"%s[DOWN]",prefix2 );
        //        }//down
        //        else if (in_msg==comp_command[7]) {
        //                switchIt("Telegram", "off");
        //                sprintf(snd_msg,"%s[OFF]",prefix2 );
        //        }//off
}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void calc_Vbat()
{
        Vbat = analogRead(A0) * Vbat_max / 1023;
        Vbat_precent = Vbat / Vbat_max * 100;
}
void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}
void setup()
{
        pinMode(A0, INPUT);
        calc_Vbat();

        // if (Vbat > Vbat_min_to_operate)
        // {

                startIOTservices();
#if USE_NOTIFY_TELE
                teleNotify.begin(telecmds);
                teleNotify.send_msg("BOOT UP");
#endif
        // }
        // else
        // {
        //         ESP.deepSleep(LowBat_sleepPeriod);
        // }
}

void addiotnalMQTT(char incoming_msg[50])
{
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0)
        {
                iot.pub_msg("empty_Status");
        }
        else if (strcmp(incoming_msg, "ver") == 0)
        {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d],ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "bat") == 0)
        {
                char battery_value[50];
                calc_Vbat();
                sprintf(msg,"Bat is %.1f, %.1f percent",Vbat, Vbat_precent);
                iot.pub_msg(msg);
        }
}
void loop()
{
        iot.looper();

#if USE_NOTIFY_TELE
        teleNotify.looper();
#endif

        delay(500);
        if (millis() >30*1000){
                ESP.deepSleep(10e6);
        }
        else{
                Serial.print("waiting...");
                Serial.println(millis());
        }


        // long now2 = millis();
        // if (now2-counter > 1000) {
        //         if(now2-counter > 1300) {
        //                 Serial.println((float)(now2-counter)/1000);
        //         }
        //         counter=now2;
        // }
}
