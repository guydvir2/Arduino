#include <Arduino.h>
#include <Ticker.h>
#include "myIOT_settings.h"
// ********** Sketch Services  ***********
#define VER "WEMOS_3.0"
#define USE_DISPLAY false
#define USE_TRAFFIC_LIGHT false

bool internetConnected = false;
bool mqttConnected = false;
bool homeAssistantConnected = false;
byte preformanceLevel = 0;
byte serviceAlertlevel = 0;

#define EVAL_PERIOD 10                 // minutes
const byte seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const byte min_ping_interval = 10;     //seconds

int adaptive_ping_val = min_ping_interval;

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

        // if (internetConnected)
        // {
        //         for (int x = 0; x < disconnectionLOG.getnumlines(); x++)
        //         {
        //                 if (readFlog(disconnectionLOG, x) > now() - time_elapsed * 3600L)
        //                 {
        //                         if (x > 0)
        //                         {
        //                                 connectedTime += (int)(readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x - 1));
        //                         }
        //                         disconncetedTime += readFlog(connectionLOG, x) - readFlog(disconnectionLOG, x);
        //                         disconnectCounter++;
        //                 }
        //         }
        //         connectedTime += now() - readFlog(connectionLOG, connectionLOG.getnumlines() - 1);
        // }
        // else
        // {
        //         for (int x = 0; x < connectionLOG.getnumlines(); x++)
        //         {
        //                 if (x > 0)
        //                 {
        //                         connectedTime += readFlog(disconnectionLOG, x) - readFlog(connectionLOG, x - 1);
        //                 }
        //                 disconncetedTime += readFlog(connectionLOG, x) - readFlog(disconnectionLOG, x);
        //                 disconnectCounter++;
        //         }
        //         disconncetedTime += now() - readFlog(disconnectionLOG, connectionLOG.getnumlines());
        // }
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

        // for (int i = 0; i < sizeof(timeFrames) / sizeof(timeFrames[0]); i++)
        // {
        //         for (int x = 0; x < sizeof(alLevels) / sizeof(alLevels[0]); x++)
        //         {
        //                 calc2(conTime, disconTime, disconnects, timeFrames[i]);
        //                 if (disconnects > alLevels[x] * timeFrames[i] || disconTime > alLevels[x] * 60 * timeFrames[i])
        //                 {
        //                         if (preformanceLevel < alLevels[x])
        //                         {
        //                                 preformanceLevel = alLevels[x];
        //                                 maxd = disconnects;
        //                                 maxc = disconTime;
        //                                 imax = i;
        //                         }
        //                 }
        //                 // ~~~~~~~~~~~~~~~~~~ End of first Test ~~~~~~~~~~~~~~~~
        //         }
        // }
        // if (lastPref != preformanceLevel)
        // {
        //         lastPref = preformanceLevel;

        //         convert_epoch2clock(maxc, 0, times, days);
        //         sprintf(msg, "Network Status: [%s], Internet [%s] MQTTserver [%s] HomeAssist[%s] Worst State: [%s] offline [%s %s] Disconnects[%d]",
        //                 State_disp[preformanceLevel], internetConnected ? "OK" : "BAD", mqttConnected ? "OK" : "BAD", homeAssistantConnected ? "OK" : "BAD",
        //                 times_disp[preformanceLevel], days, times, maxd);

        //         preformance_alert(msg);
        // }
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
                // updatePreformance();
                lastCheck = millis();
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
                // checknLOG_internet(internet_ping);
                // updateServices_Alerts();
        }
}
void checknLOG_internet(bool get_ping)
{
        static int same_state_counter = 0;
        static bool lastConStatus = false;

        // if (get_ping != lastConStatus) // change state
        // {
        //         if (get_ping == true) // internet is back on
        //         {
        //                 // writeFlog(connectionLOG, now(), true);
        //                 // internetConnected = true;
        //                 // updatePreformance();

        //                 // if (disconnectionLOG.getnumlines() > 1) // not first boot
        //                 // {
        //                 //         int offline_duration = readFlog(connectionLOG, connectionLOG.getnumlines() - 1) - readFlog(disconnectionLOG, disconnectionLOG.getnumlines() - 1);
        //                 //         if (offline_duration > seconds_offline_alarm)
        //                 //         {
        //                 //                 char tempmsg[50];
        //                 //                 sprintf(tempmsg, "Internet back ON after [%d sec]", offline_duration);
        //                 //                 iot.pub_log(tempmsg);
        //                 //         }
        //                 // }
        //                 else
        //                 {
        //                         // Serial.println("First time connection");
        //                 }
        //         }
        //         else // is now disconnected
        //         {
        //                 writeFlog(disconnectionLOG);
        //                 internetConnected = false;
        //         }
        //         lastConStatus = get_ping;
        //         same_state_counter = 0;
        //         adaptive_ping_val = min_ping_interval;
        // }
        // // No channges is connect status //
        // else
        // {
        //         if (same_state_counter < 11)
        //         {
        //                 same_state_counter++;
        //         }
        //         if (same_state_counter >= 10 && adaptive_ping_val != 6 * min_ping_interval)
        //         {
        //                 adaptive_ping_val = lastConStatus ? 6 * min_ping_interval : min_ping_interval;
        //         }
        //         if (get_ping == false && disconnectionLOG.getnumlines() == 0)
        //         {
        //                 writeFlog(disconnectionLOG);
        //         }
        // }
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
        while (x < 4)
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
        blinker.attach(0.2, TrafficBlink);
}

#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
        // startRead_parameters();
        startIOTservices();
        // endRead_parameters();
        // startFlogs();
}

void loop()
{
        iot.looper();

        pingServices();

        delay(100);
}
