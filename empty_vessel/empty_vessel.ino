#include <myIOT.h>
#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.2"
#define USE_NOTIFY_TELE true
#define ALLOW_SLEEP true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true
#define USE_WDT true
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

float Vbat = 0;
float Vbat_precent = 0;
float Vbat_max = 4.3;
float Vbat_min = 2.8;

bool use_sleep = ALLOW_SLEEP;
int LowBat_sleepPeriod = 100; // in micro-seconds
int sleepPeriod_1  = 45 * 60;   // seconds
int on_untillSleep = 40; // seconds

// ~~~~~~~ Using Telegram ~~~~~~~~~~~~~~
char *Telegram_Nick = DEVICE_TOPIC;
int time_check_messages = 2; //sec
#if USE_NOTIFY_TELE
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
        String command_set[] = {"whois_online", "status", "reset", "whoami", "help", "bat_level", "sleep_now", "use_sleep"};
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
        else if (in_msg == comp_command[5])
        {
                calc_Vbat();
                sprintf(snd_msg, "Battery is %.2f [v]", Vbat);
        }
        else if (in_msg == comp_command[6])
        {
                sprintf(snd_msg, "Sending a Sleep Command");
                ESP.deepSleep(LowBat_sleepPeriod, WAKE_NO_RFCAL);
                delay(100);
        }
        else if (in_msg == comp_command[7])
        {
                use_sleep = !use_sleep;
                sprintf(snd_msg, "use Sleep is set to %s", use_sleep ? "On" : "Off");
        }
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

BLYNK_READ(V4)
{
        Blynk.virtualWrite(V4, millis() / 1000);
}

BLYNK_READ(V5)
{
        calc_Vbat();
        Blynk.virtualWrite(V5, Vbat);
}

char auth[] = "yyJsC24RBVrsgts59QoZ_LYWj1ZEfx74";
char ssid[] = "Xiaomi_D6C8";
char pass[] = "guyd5161";

char tmsg[50];

void setup()
{
        pinMode(A0, INPUT);
        calc_Vbat();
        Blynk.begin(auth, ssid, pass);

        Blynk.begin(auth, ssid, pass);

        if ((Vbat > Vbat_min && use_sleep) || use_sleep == false)
        {
                startIOTservices();
                Serial.println("BEGIN");
                Serial.print("VALUE IS: ");
                Serial.println(use_sleep);
#if USE_NOTIFY_TELE
                teleNotify.begin(telecmds);
                iot.get_timeStamp();
                sprintf(tmsg, "[%s] %s power up , for %d seconds, with %.2f[v]",
                        iot.timeStamp, DEVICE_TOPIC, on_untillSleep, Vbat);
                teleNotify.send_msg(tmsg);
#endif
        }
        else if (Vbat < Vbat_min && use_sleep)
        {
                ESP.deepSleep(LowBat_sleepPeriod*1e6, WAKE_NO_RFCAL);
                delay(10);
        }
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
                sprintf(msg, "Bat is %.1f, %.1f percent", Vbat, Vbat_precent);
                iot.pub_msg(msg);
        }
}
void loop()
{
        iot.looper();
        calc_Vbat();
        if (Vbat < Vbat_min && use_sleep)
        {
                iot.get_timeStamp();
                sprintf(tmsg, "[%s] Low Bat %.2f- Power down", iot.timeStamp, Vbat);
                teleNotify.send_msg(tmsg);
                ESP.deepSleep(sleepPeriod_1, WAKE_NO_RFCAL);
                delay(10);
        }

#if USE_NOTIFY_TELE
        teleNotify.looper();
#endif
        if (millis() >= on_untillSleep*1000 && use_sleep)
        {
                iot.get_timeStamp();
                sprintf(tmsg, "[%s] going to sleep for %d seconds", iot.timeStamp, sleepPeriod_1);
                teleNotify.send_msg(tmsg);
                ESP.deepSleep(sleepPeriod_1 * 1e6 - micros(), WAKE_NO_RFCAL);
                delay(10);
        }

        Blynk.run();

        delay(100);
}

// void setup()
// {
//         Serial.begin(74880);
//         Serial.setTimeout(2000);
//         while (!Serial)
//         {
//         }
//         Serial.println("Started.");
//         delay(5000);
//         Serial.println("Going to sleep.");
//         ESP.deepSleep(30e6);
// }

// void loop() {}