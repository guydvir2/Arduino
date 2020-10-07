#include <myIOT.h>
#include "internet_param.h"
#include <Arduino.h>
#include <myDisplay.h>
#include <Ticker.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_2.6"
#define USE_DISPLAY false
#define USE_TRAFFIC_LIGHT true
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
bool mqttConnected = false;
bool homeAssistantConnected = false;
int alertLevel = 0;
int connAlert = 0;

const byte seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const byte min_ping_interval = 10;     //seconds

int adaptive_ping_val = min_ping_interval;
time_t begin_monitor_clock;

// ~~~~~~~~~~~~~~~~~~~ FlashLogs ~~~~~~~~~~~~~~~~~~~~
#define flosgSize 200
flashLOG connectionLOG("/conlog.txt");
flashLOG disconnectionLOG("/disconlog.txt");

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
        time_t t = now();

        if (year(t) == 1970)
        {
                iot.pub_log("NTP is not set - not entering logs entries");
        }
        else
        {
                sprintf(c, "%d", value);
                LOG.write(c);
        }
}
void deleteFlog(flashLOG &LOG)
{
        LOG.delog();
}
void startFlogs()
{
        connectionLOG.start(flosgSize, 50);
        disconnectionLOG.start(flosgSize, 50);

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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
                                connectedTime += readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x);
                                disconncetedTime += readFlog(connectionLOG, x + 1) - readFlog(disconnectionLOG, x);
                                disconnectCounter++;
                        }
                }
                connectedTime += now() - readFlog(connectionLOG, connectionLOG.getnumlines() - 1);
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
                        if (readFlog(connectionLOG, x + 1) > now() - time_elapsed * 3600L)
                        {
                                disconncetedTime += readFlog(connectionLOG, x + 1) - readFlog(disconnectionLOG, x);
                        }
                        disconncetedTime += now() - readFlog(disconnectionLOG, disconnectionLOG.getnumlines() - 1);
                }
        }
        disconnectCounter--;
}
void pingAlerts()
{
        if (mqttConnected == false || homeAssistantConnected == false)
        {
                connAlert = 2;
        }
        else if (internetConnected == false)
        {
                connAlert = 1;
        }
        else
        {
                connAlert = 0;
        }
}
void evalute_connectivity_status()
{
        // ~~~~~~~ First Test : Disconnections over Time ~~~~~~~~~~~~
        int conTime = 0;
        int disconTime = 0;
        int disconnects = 0;
        int maxd = 0;
        int maxc = 0;
        int imax = 0;

        char msg[200];
        char days[20];
        char times[20];

        int timeFrames[4] = {7 * 24, 24, 12, 1};
        char *times_disp[4] = {"Week", "Day", "12Hours", "1Hour"};
        char *State_disp[4] = {"Excellent", "Good", "notGood", "VeryBad"};

        int alLevels[] = {3, 2, 1, 0};

        for (int i = 0; i < sizeof(timeFrames) / sizeof(timeFrames[0]); i++)
        {
                for (int x = 0; x < sizeof(alLevels) / sizeof(alLevels[0]); x++)
                {
                        calc2(conTime, disconTime, disconnects, timeFrames[i]);
                        if (disconnects > alLevels[x] * timeFrames[i] || disconTime > alLevels[x] * 60 * timeFrames[i])
                        {
                                if (alertLevel < alLevels[x])
                                {
                                        alertLevel = alLevels[x];
                                        maxd = disconnects;
                                        maxc = disconTime;
                                        imax = i;
                                }
                        }
                }
        }
        // ~~~~~~~~~~~~~~~~~~ End of first Test ~~~~~~~~~~~~~~~~

        convert_epoch2clock(maxc, 0, times, days);
        sprintf(msg, "Network Status: [%s], Internet [%s] MQTTserver [%s] HomeAssist[%s] Worst State: [%s] offline [%s %s] Disconnects[%d]",
                State_disp[alertLevel], internetConnected ? "OK" : "BAD", mqttConnected ? "OK" : "BAD", homeAssistantConnected ? "OK" : "BAD",
                times_disp[alertLevel], days, times, maxd);

        if (alertLevel == 3)
        {
                iot.pub_ext(msg);
                iot.pub_msg(msg);
        }
        else if (alertLevel == 2)
        {
                iot.pub_msg(msg);
                iot.pub_log(msg);
        }
        else if (alertLevel == 1)
        {
                iot.pub_log(msg);
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
                time_t moni = readFlog(connectionLOG, 0);
                convert_epoch2clock(now(), moni, times_char[1], days_char[1]);
                epoch2datestr(moni, datestr);
                convert_epoch2clock(now(), begin_monitor_clock, times_char[2], days_char[2]);
                sprintf(msg, "Status: Internet[%s] MQTTserver[%s] HomeAssistant[%s] Disconnects[%d] monitoringTime [%s %s], First log[%s] onTime[%s %s]",
                        internetConnected ? "OK" : "FAIL", mqttConnected ? "OK" : "FAIL", homeAssistantConnected ? "OK" : "FAIL",
                        disconnectionLOG.getnumlines() - 1, days_char[1], times_char[1], datestr, days_char[2], times_char[2]);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                sprintf(msg, "ver #2: [%s], LCDdisplay[%d]", VER, USE_DISPLAY);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
                sprintf(msg, "Help #2: Commands #3 - [disconnects,[x],[y] {[x=num] [y=h,d,w]}, disconnects_log, del_disc_log]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "del_disc_log") == 0)
        {
                deleteFlog(connectionLOG);
                deleteFlog(disconnectionLOG);
                iot.pub_log("Connection logs - Deleted");
                iot.sendReset("logs deletion");
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
                                // sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(disconnectionLOG, i) -readFlog(connectionLOG, i+1));
                                sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(connectionLOG, i + 1) - readFlog(disconnectionLOG, i));
                                iot.pub_debug(log);
                        }
                }
                else
                {
                        for (int i = 0; i < disconnectionLOG.getnumlines(); i++)
                        {
                                epoch2datestr(readFlog(disconnectionLOG, i), tstamp);
                                // if (i == 0)
                                // {
                                //         sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, now() - readFlog(disconnectionLOG, i));
                                // }
                                // else
                                // {
                                //         sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(disconnectionLOG, i) - readFlog(connectionLOG, i - 1));
                                // }
                                if (i == disconnectionLOG.getnumlines() - 1)
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

                        // int x = connectionLOG.getnumlines();
                        // int y = disconnectionLOG.getnumlines();
                        // Serial.println("ConnectionLog");
                        // for (int i = 0; i < x; i++)
                        // {
                        //         Serial.printf("%d: %d\n", i, readFlog(connectionLOG, i));
                        // }
                        // Serial.println("DisConnectionLog");
                        // for (int i = 0; i < y; i++)
                        // {
                        //         Serial.printf("%d: %d\n", i, readFlog(disconnectionLOG, i));
                        // }
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

void pingServers()
{
        byte retries = 0;
        bool internet_ping = false;
        static long last_check = 0;

        if (millis() - last_check >= adaptive_ping_val * 1000L)
        {
                // long tic = millis();
                while (internet_ping == false && retries < 3 && WiFi.status() == WL_CONNECTED)
                {
                        internet_ping = iot.checkInternet("www.google.com", 2);
                        mqttConnected = iot.checkInternet("192.168.3.200", 2);
                        homeAssistantConnected = iot.checkInternet("192.168.3.199", 2);
                        retries++;
                }
                // if (millis() - tic > 4000)
                // {
                //         Serial.print("ping: ");
                //         Serial.println(millis() - tic);
                // }

                last_check = millis();
                // tic = millis();
                check_internet_changeStatus(internet_ping);
                pingAlerts();
                // if (millis() - tic > 10)
                // {
                //         Serial.print("monitor:  ");
                //         Serial.println(millis() - tic);
                // }
                // tic = millis();
        }
}
void check_internet_changeStatus(bool get_ping)
{
        static int same_state_counter = 0;
        static bool lastConStatus = false;

        if (get_ping != lastConStatus) // change state
        {
                if (get_ping == true) // internet is back on
                {
                        writeFlog(connectionLOG);
                        internetConnected = true;
                        evalute_connectivity_status();

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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~ TrafficLight ~~~~~~~~~~~~~
#define GreenLedPin D3
#define YellowLedPin D2
#define RedLedPin D1
#define ledON HIGH
#define ledOFF !ledON
#define EVAL_PERIOD 10 // minutes

Ticker blinker;
void TrafficBlink()
{
        if (connAlert == 2)
        {
                digitalWrite(RedLedPin, !digitalRead(RedLedPin));
        }
        else if (connAlert == 1)
        {
                digitalWrite(RedLedPin, ledON);
        }
        else
        {
                digitalWrite(RedLedPin, ledOFF);
        }

        if (alertLevel == 3)
        {
                digitalWrite(YellowLedPin, !digitalRead(YellowLedPin));
                digitalWrite(GreenLedPin, ledOFF);
        }
        else if (alertLevel == 2)
        {
                digitalWrite(YellowLedPin, ledON);
                digitalWrite(GreenLedPin, ledON);
        }
        else if (alertLevel == 1)
        {
                digitalWrite(YellowLedPin, ledOFF);
                digitalWrite(GreenLedPin, !digitalRead(GreenLedPin));
        }
        else if (alertLevel == 0 && connAlert ==0)
        {
                digitalWrite(YellowLedPin, ledOFF);
                digitalWrite(GreenLedPin, ledON);
        }
}
void startTrafficLight()
{
        pinMode(GreenLedPin, OUTPUT);
        pinMode(YellowLedPin, OUTPUT);
        pinMode(RedLedPin, OUTPUT);
        blinker.attach(0.2, TrafficBlink); // Start WatchDog
}
void TrafficLight_looper()
{
        static long lastCheck = 0;
        if (millis() - lastCheck > 1000 * 60L * EVAL_PERIOD)
        {
                evalute_connectivity_status();
                lastCheck = millis();
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
        startIOTservices();
        startFlogs();
        startTrafficLight();
        begin_monitor_clock = now() - (int)(millis() / 1000);

#if USE_DISPLAY
        LCD.start();
#endif
}
void loop()
{
        iot.looper();
        disconnectionLOG.looper();
        connectionLOG.looper();

        pingServers();
        TrafficLight_looper();

#if USE_DISPLAY
        gen_report_LCD();
#endif
        delay(100);
}