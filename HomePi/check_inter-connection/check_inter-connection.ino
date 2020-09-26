#include <myIOT.h>
#include "internet_param.h"
#include <Arduino.h>
#include <myDisplay.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_2.4"
#define USE_DISPLAY false
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
#define flosgSize 200
flashLOG connectionLOG("/conlog.txt");
flashLOG disconnectionLOG("/disconlog.txt");

bool internetConnected = false;
bool mqttConnected = false;
bool homeAssistantConnected = false;

const byte seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const byte min_ping_interval = 10;     //seconds

int adaptive_ping_val = min_ping_interval;
time_t begin_monitor_clock;
time_t readFlog(flashLOG &LOG, int numLine)
{
        char a[12];
        time_t ret = 0;
        if (LOG.readline(numLine, a))
        {
                ret = atoi(a);
        }
        return ret;
}
void writeFlog(flashLOG &LOG, time_t value = now())
{
        char c[12];
        // time_t t;

        // if (year(t) == 1970)
        // {
        //         iot.pub_msg("NTP is not set - not entering logs entries");
        // }
        // else
        // {
        sprintf(c, "%d", value);
        LOG.write(c);
        // }
}
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
void calc2(int &connectedTime, int &disconncetedTime, int &disconnectCounter, int time_elapsed)
{
        connectedTime = 0;
        disconncetedTime = 0;
        disconnectCounter = 0;

        if (internetConnected)
        {
                for (int x = 0; x < disconnectionLOG.getnumlines(); x++)
                {
                        if (readFlog(disconnectionLOG, x) > now() - time_elapsed * 3600L)
                        {
                                connectedTime += readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x + 1);
                                disconncetedTime += readFlog(connectionLOG, x) - readFlog(disconnectionLOG, x);
                                disconnectCounter++;
                        }
                }
                connectedTime += now() - readFlog(connectionLOG, 0);
        }
        else
        {
                for (int x = 0; x < connectionLOG.getnumlines(); x++)
                {
                        if (readFlog(disconnectionLOG, x) > now() - time_elapsed * 3600L)
                        {
                                connectedTime += readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x);
                                disconnectCounter++;
                        }
                        if (readFlog(disconnectionLOG, x + 1) > now() - time_elapsed * 3600L)
                        {
                                disconncetedTime += readFlog(connectionLOG, x) - readFlog(disconnectionLOG, x + 1);
                        }
                        disconncetedTime += now() - readFlog(disconnectionLOG, 0);
                }
        }
}

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
                char days_char[2][20];
                char times_char[2][20];
                char datestr[50];
                time_t moni = readFlog(connectionLOG, connectionLOG.getnumlines() - 1);
                convert_epoch2clock(now(), moni, times_char[1], days_char[1]);
                epoch2datestr(moni, datestr);
                convert_epoch2clock(now(), begin_monitor_clock, times_char[2], days_char[2]);
                sprintf(msg, "Status: Internet[%s] MQTTserver[%s] HomeAssistant[%s] Disconnects[%d] monitorTime [%s %s], First log[%s] onTime[%s %s]",
                        internetConnected ? "OK" : "FAIL", mqttConnected ? "OK" : "FAIL", homeAssistantConnected ? "OK" : "FAIL",
                        disconnectionLOG.getnumlines(), days_char[1], times_char[1], datestr, days_char[2], times_char[2]);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                sprintf(msg, "ver #2: [%s], TelegramServer[%d] LCDdisplay[%d]", VER, USE_TELEGRAM, USE_DISPLAY);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
                sprintf(msg, "Help #2: Commands #3 - [disconnects_1h, disconnects_24h, disconnects_1w, disconnects_log]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "disconnects_log") == 0)
        {
                char log[100];
                char tstamp[20];

                iot.pub_debug("~~~Start~~~");
                if (internetConnected)
                {
                        for (int i = 0; i < disconnectionLOG.getnumlines(); i++)
                        {
                                epoch2datestr(readFlog(disconnectionLOG, i), tstamp);
                                sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(connectionLOG, i) - readFlog(disconnectionLOG, i));
                                iot.pub_debug(log);
                        }
                }
                else
                {
                        for (int i = 0; i < disconnectionLOG.getnumlines(); i++)
                        {
                                epoch2datestr(readFlog(disconnectionLOG, i), tstamp);
                                if (i == 0)
                                {
                                        sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, now() - readFlog(disconnectionLOG, i));
                                }
                                else
                                {
                                        sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(disconnectionLOG, i) - readFlog(connectionLOG, i - 1));
                                }
                                iot.pub_debug(log);
                        }
                }
                iot.pub_msg("log extracted");
                iot.pub_debug("~~~End~~~");
        }
        // ±±±±±±±±±± MQTT MSGS ±±±±±±±±±±±±
        else
        {
                int num_parameters = iot.inline_read(incoming_msg);
                if (strcmp(iot.inline_param[0], "disconnects") == 0 && (strcmp(iot.inline_param[2], "h") == 0 || strcmp(iot.inline_param[2], "d") == 0 || strcmp(iot.inline_param[2], "w")))
                {
                        const int d = 24;
                        const int w = d * 7;

                        int counter = 0;
                        int conTime = 0;
                        int disconTime = 0;
                        char time_chars[3][20];
                        char days_chars[3][20];

                        if (strcmp(iot.inline_param[2], "h") == 0)
                        {
                                calc2(conTime, disconTime, counter, atoi(iot.inline_param[1]));
                        }
                        else if (strcmp(iot.inline_param[2], "d") == 0)
                        {
                                calc2(conTime, disconTime, counter, d * atoi(iot.inline_param[1]));
                        }
                        else if (strcmp(iot.inline_param[2], "w") == 0)
                        {
                                calc2(conTime, disconTime, counter, w * atoi(iot.inline_param[1]));
                        }

                        convert_epoch2clock(disconTime, 0, time_chars[1], days_chars[1]);

                        sprintf(msg, "Disconnects: lookupTime [%d%s] disconnectCounter [%d], offline [%s %s] ratio [%.1f%%]",
                                atoi(iot.inline_param[1]), iot.inline_param[2], counter, days_chars[1],
                                time_chars[1], 100.0 * (float)conTime / ((float)disconTime + (float)conTime));
                        iot.pub_msg(msg);

                        int x = connectionLOG.getnumlines();
                        int y = disconnectionLOG.getnumlines();
                        Serial.println("ConnectionLog");
                        for (int i = 0; i < x; i++)
                        {
                                Serial.printf("%d: %d\n", i, readFlog(connectionLOG, i));
                        }
                        Serial.println("DisConnectionLog");
                        for (int i = 0; i < y; i++)
                        {
                                Serial.printf("%d: %d\n", i, readFlog(disconnectionLOG, i));
                        }
                }
        }
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
                all_network_ok = homeAssistantConnected && mqttConnected && internetConnected;
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
#if USE_DISPLAY
                LCD.lcd.clear();
                LCD.freeTXT(line1, line2, line3, line4);
#endif
        }
}

void testConnectivity()
{
        byte retries = 0;
        bool internet_ping = false;
        static long last_check = 0;

        if (millis() - last_check >= adaptive_ping_val * 1000L)
        {
                while (internet_ping == false && retries < 3 && WiFi.status() == WL_CONNECTED)
                {
                        internet_ping = iot.checkInternet("www.google.com", 2);
                        mqttConnected = iot.checkInternet("192.168.3.200", 2);
                        homeAssistantConnected = iot.checkInternet("192.168.3.199", 2);
                        retries++;
                }

                last_check = millis();
                internetMonitor(internet_ping);
        }
}
void internetMonitor(bool get_ping)
{
        static int same_state_counter = 0;
        static bool lastConStatus = false;

        if (get_ping != lastConStatus) // change state
        {
                if (get_ping == true) // internet is back on
                {
                        writeFlog(connectionLOG);
                        internetConnected = true;
                        if (disconnectionLOG.getnumlines() > 0) // not first boot
                        {
                                Serial.println("Reconnect");
                                if (readFlog(connectionLOG, 0) - readFlog(disconnectionLOG, 0) > seconds_offline_alarm)
                                {
                                        char tempmsg[50];
                                        sprintf(tempmsg, "Internet back ON after [%d sec]", readFlog(connectionLOG, 0) - readFlog(disconnectionLOG, 0));
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
                        writeFlog(disconnectionLOG);
                        internetConnected = false;
                        // disconnect_counter++;
                        // Serial.printf("Disconnect #%d\n", disconnect_counter);
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
                if (same_state_counter >= 10 && adaptive_ping_val != 6 * min_ping_interval)
                {
                        adaptive_ping_val = lastConStatus ? 6 * min_ping_interval : min_ping_interval;
                }
        }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~ Use Telegram Server ~~~~~~~~~~~
#if USE_TELEGRAM
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
#endif

void startFlogs()
{
        connectionLOG.start(flosgSize, 12);
        disconnectionLOG.start(flosgSize, 12);
        time_t t = now();

        if (connectionLOG.getnumlines() > 0 && disconnectionLOG.getnumlines() > 0) // have both logs with entries
        {
                if (connectionLOG.getnumlines() == disconnectionLOG.getnumlines() + 1) // wakeup after it was connected
                {
                        writeFlog(disconnectionLOG); // adding an disconnect entry
                        iot.pub_log("Disconnect LOG entry was added at boot");
                }
                else if (connectionLOG.getnumlines() - disconnectionLOG.getnumlines() > 1 || disconnectionLOG.getnumlines() > connectionLOG.getnumlines())
                {
                        iot.pub_log("LOGs are not balanced- consider deleting");
                }
        }
        else if (connectionLOG.getnumlines() == 0 && disconnectionLOG.getnumlines() == 00)
        {
                writeFlog(disconnectionLOG);
                writeFlog(connectionLOG);
                iot.pub_log("Starting with both logs empty");
        }
        else if (year(t) == 1970)
        {
                iot.pub_log("NTP is not set. Can affect on log entries");
        }
}
void setup()
{
        startIOTservices();
        startFlogs();
        begin_monitor_clock = now() - (int)(millis() / 1000);

#if USE_DISPLAY
        LCD.start();
#endif

#if USE_TELEGRAM
        teleNotify.begin(telecmds);
        iot.pub_ext("BootUp");
#endif
        // connectionLOG.delog();
        // disconnectionLOG.delog();
        // writeFlog(connectionLOG, now() - 45);
        // writeFlog(disconnectionLOG, now() - 30);
        // writeFlog(connectionLOG, now() - 10);
        // writeFlog(disconnectionLOG, now() - 5);
        // writeFlog(connectionLOG, now());
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