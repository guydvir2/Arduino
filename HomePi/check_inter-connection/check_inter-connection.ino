#include <myIOT.h>
#include <Arduino.h>
#include <myDisplay.h>
#include <Ticker.h>
#include "internet_param.h"

// ********** Sketch Services  ***********
#define VER "WEMOS_2.7"
#define USE_DISPLAY false
#define USE_TRAFFIC_LIGHT true
struct MQTT_msg
{
        char from_topic[50];
        char msg[150];
        char device_topic[50];
};

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
#define USE_DEBUG true
#define DEBUG_LEVEL 0
#define USE_EXT_TOPIC true;
#define USE_NET_RESET false;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "internetMonitor"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define EXT_TOPIC "myHome/Telegram"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;
// ***************************

bool internetConnected = false;
bool mqttConnected = false;
bool homeAssistantConnected = false;
int preformanceLevel = 0;
int serviceAlertlevel = 0;

char aa[200];
#define EVAL_PERIOD 10                 // minutes
const byte seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const byte min_ping_interval = 10;     //seconds

int adaptive_ping_val = min_ping_interval;

// ~~~~~~~~~~~~~~~~~~~ FlashLogs ~~~~~~~~~~~~~~~~~~~~
#define flosgSize 200
flashLOG connectionLOG("/conlog.txt");
flashLOG disconnectionLOG("/disconlog.txt");

time_t readFlog(flashLOG &LOG, int numLine)
{
        char a[14];
        time_t ret = 0;

        if (LOG.readline(numLine, a))
        {
                ret = atoi(a);
        }
        return ret;
}
void writeFlog(flashLOG &LOG, time_t value = now(), bool writenow = false)
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
        if (writenow)
        {
                LOG.writeNow();
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
        // After reboot when connected
        if (connectionLOG.getnumlines() == disconnectionLOG.getnumlines())
        {
                writeFlog(disconnectionLOG);
        }
        // After reboot when non-connected
        else if (disconnectionLOG.getnumlines() == connectionLOG.getnumlines() + 1)
        {
                writeFlog(connectionLOG);
        }
        else
        {
                char a[50];
                sprintf(a, "Error: Connect Etries [%d] Disconnect entreis [%d]", connectionLOG.getnumlines(), disconnectionLOG.getnumlines());
                iot.pub_log(a);
        }

        if (year(t) == 1970)
        {
                iot.pub_log("NTP is not set. Can affect on log entries");
        }
}
void loopFlogs()
{
        disconnectionLOG.looper();
        connectionLOG.looper();
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
                                if (x > 0)
                                {
                                        connectedTime += (int)(readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x - 1));
                                }
                                disconncetedTime += readFlog(connectionLOG, x) - readFlog(disconnectionLOG, x);
                                disconnectCounter++;
                        }
                }
                connectedTime += now() - readFlog(connectionLOG, connectionLOG.getnumlines() - 1);
        }
        else
        {
                for (int x = 0; x < connectionLOG.getnumlines(); x++)
                {
                        if (x > 0)
                        {
                                connectedTime += readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x - 1);
                        }
                        disconncetedTime += readFlog(connectionLOG, x) - readFlog(disconnectionLOG, x);
                        disconnectCounter++;
                }
                disconncetedTime += now() - readFlog(disconnectionLOG, connectionLOG.getnumlines());
        }
}

// ~~~~~~~~~~~Update Alert Levels ~~~~~~~~~~~
void updateServices_Alerts()
{
        if (mqttConnected == false || homeAssistantConnected == false)
        {
                serviceAlertlevel = 2;
        }
        else if (internetConnected == false)
        {
                serviceAlertlevel = 1;
        }
        else
        {
                serviceAlertlevel = 0;
        }
}
void updatePreformance()
{
        static int lastPref = -1;
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

        int timeFrames[] = {7 * 24, 24, 12, 1};
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
                                if (preformanceLevel < alLevels[x])
                                {
                                        preformanceLevel = alLevels[x];
                                        maxd = disconnects;
                                        maxc = disconTime;
                                        imax = i;
                                }
                        }
                        // ~~~~~~~~~~~~~~~~~~ End of first Test ~~~~~~~~~~~~~~~~
                }
        }
        if (lastPref != preformanceLevel)
        {
                lastPref = preformanceLevel;

                convert_epoch2clock(maxc, 0, times, days);
                sprintf(msg, "Network Status: [%s], Internet [%s] MQTTserver [%s] HomeAssist[%s] Worst State: [%s] offline [%s %s] Disconnects[%d]",
                        State_disp[preformanceLevel], internetConnected ? "OK" : "BAD", mqttConnected ? "OK" : "BAD", homeAssistantConnected ? "OK" : "BAD",
                        times_disp[preformanceLevel], days, times, maxd);

                preformance_alert(msg);
        }
}
void preformance_alert(char *msg)
{
        if (preformanceLevel == 3)
        {
                iot.pub_ext(msg);
                iot.pub_msg(msg);
        }
        else if (preformanceLevel == 2)
        {
                iot.pub_msg(msg);
                iot.pub_log(msg);
        }
        else if (preformanceLevel == 1)
        {
                iot.pub_log(msg);
        }
}
void prefomance_looper()
{
        static long lastCheck = 0;
        if (millis() - lastCheck > 1000 * 60L * EVAL_PERIOD)
        {
                updatePreformance();
                lastCheck = millis();
        }
}

// ~~~~~~~~~~~~~~ myIOT ~~~~~~~~~~~~~~
void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        iot.useDebug = USE_DEBUG;
        iot.debug_level = DEBUG_LEVEL;
        iot.useextTopic = USE_EXT_TOPIC;
        iot.useNetworkReset = USE_NET_RESET;
        strcpy(iot.deviceTopic, DEVICE_TOPIC);
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        strcpy(iot.extTopic, EXT_TOPIC);

        iot.start_services(ADD_MQTT_FUNC);
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
                convert_epoch2clock(now(), readFlog(disconnectionLOG, disconnectionLOG.getnumlines() - 1), times_char[2], days_char[2]);
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
                                sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(connectionLOG, i) - readFlog(disconnectionLOG, i));
                                iot.pub_debug(log);
                        }
                }
                else
                {
                        for (int i = 0; i < connectionLOG.getnumlines(); i++)
                        {
                                epoch2datestr(readFlog(disconnectionLOG, i), tstamp);
                                sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(connectionLOG, i) - readFlog(disconnectionLOG, i));
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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
                convert_epoch2clock(now(), readFlog(disconnectionLOG, disconnectionLOG.getnumlines() - 1), cloc, days);

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

// ~~~~~ LOW-Level inspections ~~~~~~~~~
void pingServices()
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
                checknLOG_internet(internet_ping);
                updateServices_Alerts();
        }
}
void checknLOG_internet(bool get_ping)
{
        static int same_state_counter = 0;
        static bool lastConStatus = false;

        if (get_ping != lastConStatus) // change state
        {
                if (get_ping == true) // internet is back on
                {
                        writeFlog(connectionLOG, now(), true);
                        internetConnected = true;
                        updatePreformance();

                        if (disconnectionLOG.getnumlines() > 1) // not first boot
                        {
                                int offline_duration = readFlog(connectionLOG, connectionLOG.getnumlines() - 1) - readFlog(disconnectionLOG, disconnectionLOG.getnumlines() - 1);
                                if (offline_duration > seconds_offline_alarm)
                                {
                                        char tempmsg[50];
                                        sprintf(tempmsg, "Internet back ON after [%d sec]", offline_duration);
                                        iot.pub_log(tempmsg);
                                }
                        }
                        else
                        {
                                // Serial.println("First time connection");
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
                if (get_ping == false && disconnectionLOG.getnumlines() == 0)
                {
                        writeFlog(disconnectionLOG);
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~ TrafficLight ~~~~~~~~~~~~~
#if USE_TRAFFIC_LIGHT
#define GreenLedPin D3
#define YellowLedPin D2
#define RedLedPin D1
#define ledON HIGH
#define ledOFF !ledON

Ticker blinker;
void TrafficBlink()
{
        if (serviceAlertlevel == 2)
        {
                digitalWrite(RedLedPin, !digitalRead(RedLedPin));
        }
        else if (serviceAlertlevel == 1)
        {
                digitalWrite(RedLedPin, ledON);
        }
        else
        {
                digitalWrite(RedLedPin, ledOFF);
        }

        if (preformanceLevel == 3)
        {
                digitalWrite(YellowLedPin, !digitalRead(YellowLedPin));
                digitalWrite(GreenLedPin, ledOFF);
        }
        else if (preformanceLevel == 2)
        {
                digitalWrite(YellowLedPin, ledON);
                digitalWrite(GreenLedPin, ledON);
        }
        else if (preformanceLevel == 1)
        {
                digitalWrite(YellowLedPin, ledOFF);
                digitalWrite(GreenLedPin, !digitalRead(GreenLedPin));
        }
        else if (preformanceLevel == 0 && serviceAlertlevel == 0)
        {
                digitalWrite(YellowLedPin, ledOFF);
                digitalWrite(GreenLedPin, ledON);
        }
}
void powerONbit()
{
        int x = 0;
        while (x < 3)
        {
                digitalWrite(GreenLedPin, ledON);
                delay(200);
                digitalWrite(YellowLedPin, ledON);
                delay(200);
                digitalWrite(RedLedPin, ledON);
                delay(1000);

                digitalWrite(GreenLedPin, ledOFF);
                delay(200);
                digitalWrite(YellowLedPin, ledOFF);
                delay(200);
                digitalWrite(RedLedPin, ledOFF);
                delay(1000);
                x++;
        }
}
void startTrafficLight()
{
        pinMode(GreenLedPin, OUTPUT);
        pinMode(YellowLedPin, OUTPUT);
        pinMode(RedLedPin, OUTPUT);

        powerONbit();
        blinker.attach(0.2, TrafficBlink); // Start WatchDog
}

#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
        startFlogs();
#if USE_TRAFFIC_LIGHT
        startTrafficLight();
#endif

#if USE_DISPLAY
        LCD.start();
#endif
}
void loop()
{
        iot.looper();
        loopFlogs();

        pingServices();
#if USE_TRAFFIC_LIGHT
        prefomance_looper();
#endif

#if USE_DISPLAY
        gen_report_LCD();
#endif
        delay(100);
}