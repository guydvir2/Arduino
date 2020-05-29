#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_2.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "TelegramServer"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define TELEGRAM_OUT_TOPIC "Telegram_out"
#define TELEGRAM_LISTEN_TOPIC "myHome/Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

struct MQTT_msg
{
        char from_topic[50];
        char msg[150];
        char device_topic[50];
};
MQTT_msg incoming_mqtt;
const int log_size = 5;
char LOG[log_size][150];

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
char *Telegram_Nick = DEVICE_TOPIC; //"iotTest";
int time_check_messages = 1;        //sec

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
                sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, DEVICE_TOPIC);
        } // whoami
}

void send_telegram_mqtt_msg()
{
        if (chekcTelegram_topic(TELEGRAM_LISTEN_TOPIC, incoming_mqtt))
        {
                teleNotify.send_msg2(String(incoming_mqtt.msg));
                iot.pub_msg(incoming_mqtt.msg);
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        iot.useTelegram = true;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        strcpy(iot.telegramServer, TELEGRAM_OUT_TOPIC);
        iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                iot.get_timeStamp(now());
                sprintf(msg, "Status: [%s]", iot.timeStamp);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0)
        {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0)
        {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [log]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "log") == 0)
        {
                for (int i = 0; i < log_size; i++)
                {
                        if (strcmp(LOG[i], "") != 0)
                        {
                                sprintf(msg, "log: Entry [#%d] - [%s]", i, LOG[i]);
                                iot.pub_msg(msg);
                        }
                }
        }
}
void subsribe_telegram_topic(char *topic)
{
        iot.mqttClient.subscribe(topic);
}
bool chekcTelegram_topic(char *topic, MQTT_msg &msg)
{
        if (strcmp(iot.mqqt_ext_buffer[0], topic) == 0)
        {
                sprintf(msg.from_topic, "%s", iot.mqqt_ext_buffer[0]);
                sprintf(msg.msg, "%s", iot.mqqt_ext_buffer[1]);
                sprintf(msg.device_topic, "%s", iot.mqqt_ext_buffer[2]);
                enterLOG_record(msg.msg);
                // Serial.print("topic: ");
                // Serial.println(msg.from_topic);
                // Serial.print("msg: ");
                // Serial.println(msg.msg);
                // Serial.print("dev_name: ");
                // Serial.println(msg.device_topic);

                for (int i = 0; i < 3; i++)
                {
                        sprintf(iot.mqqt_ext_buffer[i], "%s", "");
                }
                return 1;
        }
        else
        {
                return 0;
        }
}
void enterLOG_record(char *log_entry)
{
        static int log_count = 0;
        if (log_count < log_size)
        {
                sprintf(LOG[log_count], "%s", log_entry);
                log_count++;
        }
        else
        {
                for (int i = 0; i < log_size - 1; i++)
                {
                        sprintf(LOG[i], "%s", LOG[i + 1]);
                }
                sprintf(LOG[log_size - 1], "%s", log_entry);
        }
}

void setup()
{
        startIOTservices();
        subsribe_telegram_topic(TELEGRAM_LISTEN_TOPIC);
        teleNotify.begin(telecmds);
        teleNotify.send_msg("TelegramServer BootUP");
}
void loop()
{
        iot.looper();
        teleNotify.looper();
        send_telegram_mqtt_msg();

  
        delay(100);
}