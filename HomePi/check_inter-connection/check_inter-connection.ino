#include <myIOT.h>
#include "internet_param.h"
#include <Arduino.h>
#include <myDisplay.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_2.3"
#define USE_DISPLAY true
#define USE_TELEGRAM true
struct MQTT_msg
{
        char from_topic[50];
        char msg[150];
        char device_topic[50];
};

// ********** myIOT Class ***********
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;
// ***************************

bool internetConnected = false;
bool mqtt_ping = false;
bool HA_ping = false;

const byte seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const byte disconnects_1hr_alarm = 5;
const byte disconnects_24hr_alarm = 10;
const byte min_ping_interval = 10; //seconds
const byte max_ping_interval = 60; // seconds

const int logSize = 200;
time_t conLog[logSize];
time_t disconLog[logSize];
time_t begin_monitor_clock;

int disconnect_counter = 0;
int adaptive_ping_val = min_ping_interval;

void startIOTservices()
{
        startRead_parameters();

        iot.useSerial = paramJSON["useSerial"];
        iot.useWDT = paramJSON["useWDT"];
        iot.useOTA = paramJSON["useOTA"];
        iot.useResetKeeper = paramJSON["useResetKeeper"];
        iot.resetFailNTP = paramJSON["useFailhNTP"];
        iot.useDebug = paramJSON["useDebugLog"];
        iot.debug_level = paramJSON["debug_level"];
        iot.useextTopic = paramJSON["useextTopic"];
        iot.useNetworkReset = paramJSON["useNetworkReset"];
        strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
        strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
        strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
        strcpy(iot.extTopic, paramJSON["extTopic"]);

        iot.start_services(ADD_MQTT_FUNC);
        endRead_parameters();
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        if (strcmp(incoming_msg, "status") == 0)
        {
                sprintf(msg, "Status: Internet[%s] MQTTserver[%s] HA[%s] total Disconnects[%d] ", internetConnected ? "OK" : "NOT OK",
                        mqtt_ping ? "OK" : "NOT OK", HA_ping ? "OK" : "NOT OK", disconnect_counter);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                sprintf(msg, "ver #2: [%s]", VER);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
                sprintf(msg, "Help #2: Commands #3 - [disconnects_1h, disconnects_24h, disconnects_1w, disconnects_log]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "disconnects_1h") == 0)
        {
                int o = 0;
                int times = 0;
                int times2 = 0;
                char time_chars[20];
                char days_chars[20];
                calc_times(conLog, logSize, 1, o, times);
                calc_times(disconLog, logSize, 1, o, times2);
                convert_epoch2clock(times-times2, 0, time_chars, days_chars);
                sprintf(msg, "Disconnects 1h: %d, Total offline Time: [%s]", o, time_chars);
                iot.pub_msg(msg);

                Serial.println("Connection: ");
                for (int a = 0; a < logSize; a++)
                {
                        Serial.println(conLog[a]);
                }

                Serial.println("disConnection: ");
                for (int a = 0; a < logSize; a++)
                {
                        Serial.println(disconLog[a]);
                }
        }
        else if (strcmp(incoming_msg, "disconnects_24h") == 0)
        {
                int o = 0;
                int times = 0;
                char time_chars[20];
                char days_chars[20];
                calc_times(disconLog, logSize, 24, o, times);
                convert_epoch2clock(times, 0, time_chars, days_chars);
                sprintf(msg, "Disconnects 24h: %d, Total offline Time: [%s]", o, time_chars);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "disconnects_1w") == 0)
        {
                int o = 0;
                int times = 0;
                char time_chars[20];
                char days_chars[20];
                calc_times(disconLog, logSize, 24 * 7, o, times);
                convert_epoch2clock(times, 0, time_chars, days_chars);
                sprintf(msg, "Disconnects 24h: %d, Total offline Time: [%s]", o, time_chars);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "disconnects_log") == 0)
        {
                char log[100];
                char tstamp[20];

                if (internetConnected)
                {
                        for (int i = 0; i < logSize; i++)
                        {
                                if (disconLog[i] != 0)
                                {
                                        epoch2datestr(disconLog[i], tstamp);
                                        sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, conLog[i] - disconLog[i]);
                                        iot.pub_debug(log);
                                }
                        }
                }
                else
                {
                }
                iot.pub_msg("log extracted");
        }
}
void epoch2datestr(time_t t, char clockstr[50])
{
        sprintf(clockstr, "%04d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}

// ~~~~~~~~~~~ LCD Display ~~~~~~~~~~~
#if USE_DISPLAY
myLCD LCD(20, 4);
#endif

void gen_report_LCD(int refresh_time = 700)
{
        static long last_ref = 0;

        if (millis() - last_ref > refresh_time)
        {
                generate_monitor_status();
                last_ref = millis();
        }
}

void initLog(time_t log[], int sizeLog = logSize, time_t init_val = 0)
{
        for (int i = 0; i < sizeLog; i++)
        {
                log[i] = init_val;
        }
}
void update_MAG(time_t log[], int sizeLog, time_t newval)
{
        for (int a = sizeLog - 1; a > 0; a--)
        {
                log[a] = log[a - 1];
        }
        log[0] = newval;

        for (int a = 0; a < sizeLog; a++)
        {
                Serial.println(log[a]);
        }
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
        char line1[25];
        char line2[25];
        char line3[25];
        char line4[25];
        char cloc[30];
        char days[20];

        static unsigned long lastrefresh = 0;
        static bool all_network_ok = false;

        if (millis() - lastrefresh > 10000)
        {
                mqtt_ping = iot.checkInternet("192.168.3.200", 2);
                HA_ping = iot.checkInternet("192.168.3.199", 2);
                all_network_ok = HA_ping && mqtt_ping && internetConnected;
                lastrefresh = millis();
        }
        else
        {
                iot.get_timeStamp(now());
                convert_epoch2clock(now(), begin_monitor_clock, cloc, days);

                sprintf(line1, ">>Network  Monitor<<");
                sprintf(line2, iot.timeStamp);
                sprintf(line3, "upTime:%s %s", days, cloc);
                sprintf(line4, "%s", all_network_ok ? "Network OK" : "Network error");
                LCD.lcd.clear();
                LCD.freeTXT(line1, line2, line3, line4);
        }
}
void testConnectivity(int interval_check = 30)
{
        byte retries = 0;
        bool reachout = false;
        static long last_check = 0;

        if (millis() - last_check >= interval_check * 1000)
        {
                while (reachout == false && retries < 3 && WiFi.status() == WL_CONNECTED)
                {
                        reachout = iot.checkInternet("www.google.com", 2);
                        retries++;
                }
                last_check = millis();
                internetMonitor(reachout);
        }
}
void internetMonitor(bool get_ping)
{
        static int same_state_counter = 0;
        static bool lastConStatus = false;

        if (get_ping != lastConStatus) // change status
        {
                if (get_ping == true) // internet is back on
                {
                        update_MAG(conLog, logSize, now());
                        internetConnected = true;
                        if (disconLog[0] != 0) // not first boot
                        {
                                Serial.println("Reconnect");
                                // Exceed offline time - Alarm !
                                if (conLog[0] - disconLog[0] > seconds_offline_alarm)
                                {
                                        char tempmsg[50];
                                        sprintf(tempmsg, "Internet back ON after [%d sec]", conLog[0] - disconLog[0]);
                                        iot.pub_log(tempmsg);
                                }
                        }
                        else
                        {
                                Serial.println("First time connection");
                        }
                }
                else // is now disconnected
                {
                        update_MAG(disconLog, logSize, now());
                        internetConnected = false;
                        disconnect_counter++;
                        Serial.printf("Disconnect #%d\n", disconnect_counter);
                }
                lastConStatus = get_ping;
                same_state_counter = 0;
                adaptive_ping_val = min_ping_interval;
        }
        // No channges is connect status //
        else
        {
                if (same_state_counter < 11)
                {
                        same_state_counter++;
                }
                if (same_state_counter >= 10 && adaptive_ping_val != max_ping_interval)
                {
                        adaptive_ping_val = lastConStatus ? max_ping_interval : min_ping_interval;
                }
                // if (get_ping == false && inter_fail_start == 0)
                // {
                //         disconLog[0] = millis(); // wake up without clock and no onternet and NTP
                // }
        }
}
void calc_times(time_t log[], int logsize, int hours, int &occur, int &timed)
{
        occur = 0;
        timed = 0;

        for (int i = 0; i < logsize; i++)
        {
                Serial.println(log[i]);
                if (log[i] != 0 && log[i + 1] != 0)
                {
                        occur++;
                        if (log[i] > now() - hours * 3600L)
                        {
                                timed += log[i] - log[i + 1];
                        }
                }
        }
        Serial.print("Time: ");
        Serial.println(timed);
        Serial.print("Occurances: ");
        Serial.println(occur);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~ Use Telegram Server ~~~~~~~~~~~
#if USE_TELEGRAM
MQTT_msg incoming_mqtt;
char *Telegram_Nick = "TelegramServer";
byte time_check_messages = 1; //sec

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
#endif

void setup()
{

        startIOTservices();
        initLog(conLog, logSize, 0);
        initLog(disconLog, logSize, 0);
        Serial.print("now is: ");
        Serial.println(now());
        begin_monitor_clock = now() - (int)(millis() / 1000);

#if USE_DISPLAY
        LCD.start();
#endif

#if USE_TELEGRAM
        teleNotify.begin(telecmds);
        iot.pub_ext("BootUp");
#endif

        int o = 0;
        int times = 0;
        initLog(conLog);
        initLog(disconLog);
        // update_MAG(conLog, logSize, now() - 45);
        // update_MAG(disconLog, logSize, now() - 30);
        // update_MAG(conLog, logSize, now() - 15);
        // update_MAG(disconLog, logSize, now() - 10);
        // update_MAG(conLog, logSize, now());

        // calc_times(conLog, logSize, 1, o, times);
        // Serial.printf("Reconnects: %d, Total offline Time: %d[sec]\n", o, times);
        // calc_times(disconLog, logSize, 1, o, times);
        // Serial.printf("Disconnects: %d, Total online Time: %d[sec]\n", o, times);

        // initLog(conLog);
        // initLog(disconLog);
}
void loop()
{
        iot.looper();
        testConnectivity();

#if USE_DISPLAY
        gen_report_LCD();
#endif

#if USE_TELEGRAM
        teleNotify.looper();
        listenMQTT_forTelegram();
#endif

        delay(100);
}