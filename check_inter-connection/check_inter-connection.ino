#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "internetMonitor"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define TELEGRAM_OUT_TOPIC "myHome/Telegram_out"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

char monitor_string[200];
bool internetConnected = false;
const int seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const int disconnects_1hr_alarm = 5;
const int disconnects_24hr_alarm = 10;
int disc_1h[disconnects_1hr_alarm];
int disc_24hr[disconnects_24hr_alarm];
int disconnect_counter = 0;
int longest_discon = 0;
long accum_connect = 0;
long accum_disconnect = 0;
time_t begin_monitor_clock;
time_t inter_ok_start = 0;
time_t inter_fail_start = 0;

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

void sendTelegramServer(char *msg, char *tele_server = TELEGRAM_OUT_TOPIC)
{
        char t[200];
        iot.get_timeStamp(now());
        sprintf(t, "[%s][%s]: %s", iot.timeStamp, iot.deviceTopic, msg);
        iot.mqttClient.publish(tele_server, t);
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

        sprintf(days_str, "%02d days", days);
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
        }
        else
        {
                temp_disconn_accu = accum_disconnect + now() - inter_ok_start;
                temp_conn_accu = accum_connect;
        }

        convert_epoch2clock(now(), begin_monitor_clock, cloc, days);
        if (strcmp(days, "00 days") != 0)
        {
                sprintf(t1, "Duration [%s %s]", days, cloc);
        }
        else
        {
                sprintf(t1, "Total Duration [%s]", cloc);
        }

        convert_epoch2clock(temp_conn_accu, 0, cloc, days);
        if (strcmp(days, "00 days") != 0)
        {
                sprintf(t2, "Monitoring Time[%s %s]", days, cloc);
        }
        else
        {
                sprintf(t2, "Connected[%s]", cloc);
        }

        unsigned long dur = now() - begin_monitor_clock;

        float conn_ratio = (float)(temp_conn_accu / (float)dur);
        Serial.println(conn_ratio);

        sprintf(monitor_string, "Status: %s; %s; disconnections[#%d]; Longest_Disconnect[%d sec]; Connect_ratio[%.03f]",
                t1, t2, disconnect_counter, longest_discon, conn_ratio);
}
void prcoess_status(bool get_ping)
{
        static int hrly_counter = 0;
        static int daily_counter = 0;

        if (get_ping != internetConnected)
        {
                if (get_ping == true)
                {
                        if (inter_ok_start != 0)
                        {
                                // Exceed offline time - Alarm !
                                if (now() - inter_ok_start > seconds_offline_alarm)
                                {
                                        char tempmsg[50];
                                        sprintf(tempmsg, "Internet back ON after [%d sec]", now() - inter_ok_start);
                                        sendTelegramServer(tempmsg);
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
                                                sendTelegramServer(tempmsg);
                                                for (int i = 0; i < disconnects_1hr_alarm; i++)
                                                {
                                                        disc_1h[i] = 0;
                                                }
                                                hrly_counter = 0;
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
                                        if (now() - disc_1h[0] < 60 * 60 * 24)
                                        {
                                                char tempmsg[50];
                                                sprintf(tempmsg, "Exceed [%d] disconnections in 24hr, [%d secs]", disconnects_24hr_alarm, now() - disc_24hr[0]);
                                                sendTelegramServer(tempmsg);
                                                for (int i = 0; i < disconnects_24hr_alarm; i++)
                                                {
                                                        disc_24hr[i] = 0;
                                                }
                                                daily_counter = 0;
                                        }
                                }
                        }
                        inter_ok_start = now();
                        int disco_time = inter_ok_start - inter_fail_start;
                        if (disco_time > longest_discon && inter_fail_start != 0)
                        {
                                longest_discon = disco_time;
                        }
                        accum_disconnect += disco_time;
                }
                else
                {
                        inter_fail_start = now();
                        accum_connect = inter_ok_start - inter_fail_start;
                        disconnect_counter++;
                        Serial.print("Disconnect_time: ");
                        Serial.println(inter_fail_start);
                }

                internetConnected = get_ping;
        }
}
void ping_it(int interval_check = 10)
{
        static long last_check = 0;

        if (millis() - last_check >= interval_check * 1000)
        {
                bool reachout = false;
                byte retries = 0;

                while (reachout == false && retries < 2)
                {
                        reachout = iot.checkInternet("www.google.com", 2);
                        retries++;
                }
                prcoess_status(reachout);
                last_check = millis();
        }
}

void setup()
{
        startIOTservices();
        begin_monitor_clock = now();
}
void loop()
{
        iot.looper();
        ping_it();

        delay(100);
}
