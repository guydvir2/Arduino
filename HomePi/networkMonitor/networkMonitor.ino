#include <myIOT.h>
#include <Arduino.h>
#include <myDisplay.h>

// ********** Sketch Services  ***********
#define VER "NodeMCU_0.1"
#define USE_DISPLAY true
#define USE_TELEGRAM true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "networkMonitor"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define TELEGRAM_OUT_TOPIC "myHome/Telegram"
#define ADD_MQTT_FUNC addiotnalMQTT
// ***************************

myIOT iot(DEVICE_TOPIC);
struct MQTT_msg
{
        char from_topic[50];
        char msg[150];
        char device_topic[50];
};

void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        iot.useextTopic = USE_TELEGRAM;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        strcpy(iot.extTopic, TELEGRAM_OUT_TOPIC);

        iot.start_services(ADD_MQTT_FUNC);
}
char monitor_string[200];
bool internetConnected = false;
const int seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const int disconnects_1hr_alarm = 5;
const int disconnects_24hr_alarm = 10;
int disc_1h[disconnects_1hr_alarm];
int disc_24hr[disconnects_24hr_alarm];
int disconnect_counter = 0;
int longest_discon = 0;
const int min_ping_interval = 10; //seconds
const int max_ping_interval = 60; // seconds
int adaptive_ping_val = min_ping_interval;
long accum_connect = 0;
float conn_ratio = 0;
long accum_disconnect = 0;
time_t begin_monitor_clock;
time_t inter_ok_start = 0;
time_t inter_fail_start = 0;


void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        if (strcmp(incoming_msg, "status") == 0)
        {
                generate_monitor_status();
                iot.pub_msg(monitor_string);
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
                sprintf(msg, "Help: Commands #2 - [disconnect_1, disconnect_2]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "disconnect_1") == 0)
        {
                for (int i = 0; i < disconnects_1hr_alarm; i++)
                {
                        char tmpmsg[30];
                        if (disc_1h[i] != 0)
                        {
                                iot.get_timeStamp(disc_1h[i]);
                                sprintf(tmpmsg, "Hourly disconnects: [#%d] [%s]", i, iot.timeStamp);
                                iot.pub_msg(tmpmsg);
                                Serial.println(tmpmsg);
                        }
                }
        }
        else if (strcmp(incoming_msg, "disconnect_2") == 0)
        {
                for (int i = 0; i < disconnects_24hr_alarm; i++)
                {
                        char tmpmsg[30];
                        if (disc_24hr[i] != 0)
                        {
                                iot.get_timeStamp(disc_24hr[i]);
                                sprintf(tmpmsg, "24hr disconnects: [#%d] [%s]", i, iot.timeStamp);
                                iot.pub_msg(tmpmsg);
                        }
                }
        }
}
// ***************************

// ~~~~~~~~~~~ LCD Display ~~~~~~~~~~~
#if USE_DISPLAY
myLCD LCD(20, 4);

void gen_report_LCD(int refresh_time = 5000)
{
        static long last_ref = 0;

        if (millis() - last_ref > refresh_time)
        {
                generate_monitor_status();
                last_ref = millis();
        }
}
#endif

// ~~~~~~~~~~~~~ Internet Monitoring ~~~~~~~~~~
void epoch2datestr(time_t t, char clockstr[50])
{
        sprintf(clockstr, "%04d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
void convert_epoch2clock(long t1, long t2, char *time_str, char *days_str)
{
        byte days = 0;
        byte hours = 0;
        byte minutes = 0;
        byte seconds = 0;

        int sec2minutes = 60;
        int sec2hours = (sec2minutes * 60);
        int sec2days = (sec2hours * 24);
        int sec2years = (sec2days * 365);

        long time_delta = t1 - t2;

        days = (int)(time_delta / sec2days);
        hours = (int)((time_delta - days * sec2days) / sec2hours);
        minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

        sprintf(days_str, "%dd", days);
        sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
void generate_monitor_status()
{
        char cloc[30];
        char days[20];
        char t1[100];
        char t2[100];
        unsigned long temp_conn_accu;
        unsigned long temp_disconn_accu;

        // calc duration connect or disconnect
        if (internetConnected)
        {
                temp_conn_accu = accum_connect + now() - inter_ok_start;
                temp_disconn_accu = accum_disconnect;
        }
        else
        {
                temp_disconn_accu = accum_disconnect + now() - inter_ok_start;
                temp_conn_accu = accum_connect;
        }

        convert_epoch2clock(now(), begin_monitor_clock, cloc, days);
        if (strcmp(days, "0d") != 0)
        {
                sprintf(t1, "Duration [%s %s]", days, cloc);
        }
        else
        {
                sprintf(t1, "Duration [%s]", cloc);
        }

        convert_epoch2clock(temp_conn_accu, 0, cloc, days);
        if (strcmp(days, "00 days") != 0)
        {
                sprintf(t2, "Monitoring [%s %s]", days, cloc);
        }
        else
        {
                sprintf(t2, "Monitoring [%s]", cloc);
        }

        unsigned long dur = now() - begin_monitor_clock;

        conn_ratio = (float)(temp_conn_accu / (float)dur);

        sprintf(monitor_string, "Status: %s; %s; disconnections[#%d]; Longest_Disconnect[%d sec]; Connect_ratio[%.03f]",
                t1, t2, disconnect_counter, longest_discon, conn_ratio);

#if USE_DISPLAY
        char line1[25];
        char line2[25];
        char line3[25];

        iot.get_timeStamp(now());
        bool mqtt_ping = iot.checkInternet("192.168.3.200", 2);
        bool HA_ping = iot.checkInternet("192.168.3.199", 2);
        sprintf(line2, "Time: %s", iot.timeStamp);

        sprintf(line1, "internet:%s, f:%.2f%%", internetConnected ? "OK" : "Fail", (float)(temp_disconn_accu / (float)dur) * 100);
        sprintf(line2, "MQTTbrok:%s HA:%s", mqtt_ping ? "OK" : "Fail", HA_ping ? "OK" : "Fail");
        convert_epoch2clock(now(), begin_monitor_clock, cloc, days);
        sprintf(line3, "upTime:%s %s", days, cloc);

        // sprintf(line3, "tot:", disconnect_counter, longest_discon);
        LCD.lcd.clear();
        LCD.freeTXT(">>Network  Monitor<<", line1, line2, line3);

#endif
}
void prcoess_status(bool get_ping)
{
        static int hrly_counter = 0;
        static int daily_counter = 0;
        static int same_state_counter = 0;

        if (get_ping != internetConnected) // change status
        {
                if (get_ping == true) // internet is back on
                {
                        if (inter_ok_start != 0) // not first boot
                        {
                                // Exceed offline time - Alarm !
                                if (now() - inter_fail_start > seconds_offline_alarm)
                                {
                                        char tempmsg[50];
                                        sprintf(tempmsg, "Internet back ON after [%d sec]", now() - inter_fail_start);
                                        iot.pub_ext(tempmsg);
                                }

                                // Exceed amount of disconnects in 1 hr - Alarm
                                if (hrly_counter < disconnects_1hr_alarm)
                                {
                                        disc_1h[hrly_counter] = now();
                                        hrly_counter++;
                                }
                                else
                                {
                                        if (now() - disc_1h[0] < 60 * 60)
                                        {
                                                char tempmsg[50];
                                                sprintf(tempmsg, "Exceed [%d] disconnections in 1hr, [%d secs]", disconnects_1hr_alarm, now() - disc_1h[0]);
                                                iot.pub_ext(tempmsg);
                                                for (int i = 0; i < disconnects_1hr_alarm; i++)
                                                {
                                                        disc_1h[i] = 0;
                                                }
                                                hrly_counter = 0;
                                        }
                                        else
                                        {
                                                for (int i = 0; i < disconnects_1hr_alarm - 1; i++)
                                                {
                                                        disc_1h[i] = disc_1h[i + 1];
                                                }
                                                disc_1h[hrly_counter] = now();
                                        }
                                }

                                // Exceed amount of disconnects in 24 hr - Alarm
                                if (daily_counter < disconnects_24hr_alarm)
                                {
                                        disc_24hr[daily_counter] = now();
                                        daily_counter++;
                                }
                                else
                                {
                                        if (now() - disc_24hr[0] < 60 * 60 * 24)
                                        {
                                                char tempmsg[50];
                                                sprintf(tempmsg, "Exceed [%d] disconnections in 24hr, [%d secs]", disconnects_24hr_alarm, now() - disc_24hr[0]);
                                                iot.pub_ext(tempmsg);
                                                for (int i = 0; i < disconnects_24hr_alarm; i++)
                                                {
                                                        disc_24hr[i] = 0;
                                                }
                                                daily_counter = 0;
                                        }
                                        else
                                        {
                                                for (int i = 0; i < disconnects_24hr_alarm - 1; i++)
                                                {
                                                        disc_24hr[i] = disc_24hr[i + 1];
                                                }
                                                disc_24hr[daily_counter] = now();
                                        }
                                }

                                inter_ok_start = now();
                                int disco_time = inter_ok_start - inter_fail_start;
                                if (disco_time > longest_discon && inter_fail_start != 0)
                                {
                                        longest_discon = disco_time;
                                }
                                accum_disconnect += disco_time;
                                Serial.print("connect_time: ");
                                Serial.println(inter_ok_start);

                                char notif[150];
                                char clock0[50];
                                char clock1[50];
                                epoch2datestr(inter_ok_start, clock0);
                                epoch2datestr(inter_fail_start, clock1);

                                sprintf(notif, "Reconnect: [%s]; Disconnect: [%s], offline-duration: [%d sec]", clock0, clock1, inter_ok_start - inter_fail_start);
                                iot.pub_msg(notif);
                                iot.pub_ext(notif);
                        }
                        else
                        {
                                inter_ok_start = now();
                                if (inter_fail_start != 0)
                                {
                                        inter_ok_start - (int)inter_fail_start / 1000;
                                }
                        }
                }
                else // is now disconnect
                {
                        inter_fail_start = now();
                        accum_connect += inter_fail_start - inter_ok_start;
                        disconnect_counter++;
                }
                internetConnected = get_ping;
                same_state_counter = 0;
                adaptive_ping_val = min_ping_interval;
        }
        // No channges is connect status //
        else
        {
                same_state_counter++;
                if (same_state_counter >= 10 && adaptive_ping_val != max_ping_interval)
                {
                        adaptive_ping_val = internetConnected ? max_ping_interval : min_ping_interval;
                }
                if (get_ping == false && inter_fail_start == 0)
                {
                        inter_fail_start = millis(); // wake up without clock and no onternet and NTP
                }
        }
}
void ping_it(int interval_check = 30)
{
        static long last_check = 0;

        if (millis() - last_check >= interval_check * 1000)
        {
                bool reachout = false;
                byte retries = 0;

                while (reachout == false && retries < 3)
                {
                        reachout = iot.checkInternet("www.google.com", 2);
                        retries++;
                }
                prcoess_status(reachout);
                last_check = millis();
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~

MQTT_msg incoming_mqtt;
const int log_size = 5;
char LOG[log_size][150];
char *Telegram_Nick = DEVICE_TOPIC;
int time_check_messages = 1; //sec

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
bool chekcTelegram_topic(char *topic, MQTT_msg &msg)
{
        if (strcmp(iot.mqqt_ext_buffer[0], topic) == 0)
        {
                sprintf(msg.from_topic, "%s", iot.mqqt_ext_buffer[0]);
                sprintf(msg.msg, "%s", iot.mqqt_ext_buffer[1]);
                sprintf(msg.device_topic, "%s", iot.mqqt_ext_buffer[2]);
                // enterLOG_record(msg.msg);
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
        if (chekcTelegram_topic(TELEGRAM_OUT_TOPIC, incoming_mqtt))
        {
                teleNotify.send_msg2(String(incoming_mqtt.msg));
                for (int i = 0; i < 3; i++)
                {
                        sprintf(iot.mqqt_ext_buffer[i], "%s", "");
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
        startIOTservices();
        begin_monitor_clock = now() - (int)(millis() / 1000);

#if USE_TELEGRAM
        teleNotify.begin(telecmds);
        iot.pub_ext("networkMonitor Boot");
#endif

#if USE_DISPLAY
        LCD.start();
#endif

}
void loop()
{
        iot.looper();
        ping_it(adaptive_ping_val);

#if USE_DISPLAY
        gen_report_LCD();
#endif

#if USE_TELEGRAM
        teleNotify.looper();
        listenMQTT_forTelegram();
#endif
        delay(100);
}