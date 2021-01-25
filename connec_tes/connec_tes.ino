#include <Arduino.h>
#include <Ticker.h>
#include "myIOT_settings.h"

// ********** Sketch Services  ***********
#define VER "WEMOS_3.1"
#define USE_TRAFFIC_LIGHT false
#define SCAN_PREFOMANCE_INTERVAL 1 // minutes

bool internetConnected = false;
bool mqttConnected = false;
bool homeAssistantConnected = false;
byte preformanceLevel = 0;
byte serviceAlertlevel = 0;

const byte log_entries = 50;
unsigned long connLOG[log_entries];
unsigned long disconnLOG[log_entries];

#define EVAL_PERIOD 10                 // minutes
const byte seconds_offline_alarm = 60; // past this time , an Alert will be sent upon reconnection
const byte min_ping_interval = 10;     //seconds

int adaptive_ping_val = min_ping_interval;
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

        //         iot.convert_epoch2clock(maxc, 0, times, days);
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

void show_clk(time_t t = now())
{
        char clk[20];
        char days[5];
        char msg[50];
        iot.get_timeStamp(t);
        sprintf(msg, "%d, clock:%s", t, iot.timeStamp);
        Serial.println(msg);
}
bool verifyLOG()
{
        int c = getLOG_entries(connLOG);
        int d = getLOG_entries(disconnLOG);
        bool logerr = true;

        /* Check LOGS are sorted correctly by time stamp */
        for (int i = 0; i < c - 1; i++)
        {
                if (connLOG[i] > connLOG[i + 1])
                {
                        Serial.print("connect log entry #");
                        Serial.print(i);
                        Serial.println(" FAIL");
                        logerr *= false;
                }
                else
                {
                        // Serial.println("Connect log sequence OK");
                        logerr *= true;
                }
        }
        for (int i = 0; i < d - 1; i++)
        {
                if (disconnLOG[i] > disconnLOG[i + 1])
                {
                        Serial.print("disconnect log entry #");
                        Serial.print(i);
                        Serial.println(" FAIL");
                        logerr *= false;
                }
                else
                {
                        // Serial.println("disConnect log sequence OK");
                        logerr *= true;
                }
        }

        /* Check connect log size vs disconnect size*/
        if (internetConnected)
        {
                if (c == d + 1)
                {
                        for (int i = 0; i < d; i++)
                        {
                                if (connLOG[i] > disconnLOG[i])
                                {
                                        Serial.print("error in 2 sequental logs");
                                        logerr *= false;
                                }
                                else
                                {
                                        logerr *= true;
                                }
                        }
                }
                else
                {
                        Serial.println("log sizes :NOT OK");
                        logerr *= false;
                }
        }
        else
        {
                if (c == d)
                {
                        // Serial.println("log sizes :OK");
                        logerr *= true;
                }
                else
                {
                        Serial.println("log sizes :NOT OK");
                        logerr *= false;
                }
        }
        return logerr;
}
void fixLOG()
{
        int c = getLOG_entries(connLOG);
        int d = getLOG_entries(disconnLOG);

        if (internetConnected)
        {
                if (c != d + 1)
                {
                        Serial.println("connected: size err");
                }
        }
        else
        {
                if (c != d)
                {
                        Serial.println("not-connected: size err");
                }
        }
}

int calc_discon_time(int x = 0)
{
        int c = getLOG_entries(connLOG);
        int calc = -1;
        if (x > 0 && x < c)
        {
                if (verifyLOG())
                {
                        calc = connLOG[x] - disconnLOG[x - 1];
                }
                else
                {
                        Serial.println("log error");
                        calc = 0;
                }
        }
        else
        {
                Serial.println("out bound disconn log");
        }

        return calc;
}
int calc_con_time(int x = 0)
{
        int c = getLOG_entries(connLOG);
        int d = getLOG_entries(disconnLOG);
        int calc = -1;
        if (x < d)
        {
                if (verifyLOG())
                {
                        calc = disconnLOG[x] - connLOG[x];
                }
                else
                {
                        Serial.println("log error");
                        calc = 0;
                }
        }
        else if (x == c - 1)
        {
                calc = now() - connLOG[c - 1];
        }

        return calc;
}

void updateLOG(unsigned long LOG[], unsigned long now = now())
{

        driftLOG(LOG); /* If log is full- swap is done */
        byte i = getLOG_entries(LOG);
        if (now != 0 && LOG[i - 1] < now)
        {
                LOG[i] = now;
        }
        else
        {
                Serial.println("entry error for log. entry ignored");
        }
}
void printLOG(unsigned long LOG[], char *topic)
{
        char msg[40];
        char clk[20];
        char days[10];

        Serial.print(" \n*** Start ");
        Serial.print(topic);
        Serial.println(" *** ");
        for (int i = 0; i < getLOG_entries(LOG); i++)
        {
                Serial.print("entry #");
                Serial.print(i);
                Serial.print(":\t");
                Serial.print(LOG[i]);
                Serial.print("\t");
                iot.get_timeStamp(LOG[i]);
                Serial.println(iot.timeStamp);
        }
        Serial.print(" *** END ");
        Serial.print(topic);
        Serial.println(" *** ");
}
int getLOG_entries(unsigned long LOG[])
{
        int i = 0;
        while (i < log_entries)
        {
                if (LOG[i] == 0)
                {
                        break;
                }
                i++;
        }
        return i;
}
bool driftLOG(unsigned long LOG[])
{
        int i = getLOG_entries(LOG);
        if (i >= log_entries - 1)
        {
                for (int x = 0; x < i; x++)
                {
                        LOG[x] = LOG[x + 1];
                        Serial.print("drif_log_#");
                        Serial.println(x);
                }
                return 1;
        }
        return 0;
}

void calc_total_time(unsigned long &total_time)
{
        if (verifyLOG)
        {
                total_time = now() - connLOG[0];
        }
        else
        {
                total_time = 0;
                Serial.println("error total time");
        }
}
void calc_connection(unsigned long &cum_conTime, unsigned long &cum_disconTime, unsigned long &total_time, int &disconnects_counter, float timePeriod = 24)
{
        int con = getLOG_entries(connLOG);
        calc_total_time(total_time);
        unsigned long crit = now() - int(timePeriod * 3600UL);

        if (internetConnected)
        {
                Serial.print("\n~~~~~~~~~~~~~~~");
                Serial.print(timePeriod);
                Serial.print("_hrs");
                Serial.println("~~~~~~~~~~~~~~~");
                Serial.print("criteria_");
                Serial.print(timePeriod);
                Serial.print("hrs: \t\t");
                show_clk(crit);

                for (int i = 1; i < con; i++)
                {
                        if (disconnLOG[i-1] > crit)
                        {
                                cum_disconTime += calc_discon_time(i); /* Summing offline times */
                                disconnects_counter++;
                        }
                }
        }
        else
        {
                // need to handle this later
        }
}
void run_connection_report()
{
        char msg[40];
        char clk[20];
        char days[10];
        unsigned long cum_conTime = 0;
        unsigned long cum_disconTime = 0;
        unsigned long total_time = 0;
        int disconnects_counter = 0;

        float check_times[] = {0.5, 3};                            //, 12//};                                                      //, 24, 24 * 7}; // report for those times, in hrs
        int disconnect_fail_criteria[][2] = {{1, 2}, {2, 4}};      //, { 2, 4 }}; //, {3, 6}, {10, 20}};
        int disconTime_fail_criteria[][2] = {{30, 60}, {60, 120}}; //, { 60, 120 }};
        int score[2];

        for (int i = 0; i < sizeof(check_times) / sizeof(check_times[0]); i++)
        {
                calc_connection(cum_conTime, cum_disconTime, total_time, disconnects_counter, check_times[i]);

                if (disconnects_counter > disconnect_fail_criteria[i][1] || cum_disconTime > disconTime_fail_criteria[i][1])
                {
                        score[i] = 2;
                }
                else if (disconnects_counter > disconnect_fail_criteria[i][0] && disconnects_counter <= disconnect_fail_criteria[i][1])
                {
                        score[i] = 1;
                }
                else if (cum_disconTime > disconTime_fail_criteria[i][0] && cum_disconTime <= disconTime_fail_criteria[i][1])
                {
                        score[i] = 1;
                }
                else
                {
                        score[i] = 0;
                }

                iot.convert_epoch2clock(total_time, 0, clk, days);
                sprintf(msg, "Total time:\t\t\t%s %s", days, clk);
                Serial.println(msg);

                sprintf(msg, "Total disonnects:\t\t%d", getLOG_entries(disconnLOG));
                Serial.println(msg);

                // iot.convert_epoch2clock(cum_conTime, 0, clk, days);
                // sprintf(msg, "Connected in %.2f_hrs:\t%s %s", check_times[i], days, clk);
                // Serial.println(msg);

                iot.convert_epoch2clock(cum_disconTime, 0, clk, days);
                sprintf(msg, "Disonnected in %.2f_hrs:\t%s %s", check_times[i], days, clk);
                Serial.println(msg);

                sprintf(msg, "Disconnects in %.2f_hrs:\t%d", check_times[i], disconnects_counter);
                Serial.println(msg);

                sprintf(msg, "Score in %.2f_hrs:\t\t%d", check_times[i], score[i]);
                Serial.println(msg);

                disconnects_counter = 0;
                cum_conTime = 0;
                cum_disconTime = 0;
        }
}
void display_logs()
{
        int con = getLOG_entries(connLOG);
        int discon = getLOG_entries(disconnLOG);

        if (internetConnected && verifyLOG()) /* nominal: internet connected without log errors*/
        {
                Serial.println("±±±±± DISCONNECT ±±±±±");
                for (int i = 1; i <= discon; i++)
                {
                        char clk[3][20];
                        char days[10];
                        char msg[150];
                        unsigned int a = calc_discon_time(i);
                        iot.get_timeStamp(connLOG[i]);
                        sprintf(clk[0], iot.timeStamp);
                        iot.get_timeStamp(disconnLOG[i - 1]);
                        sprintf(clk[1], iot.timeStamp);

                        iot.convert_epoch2clock(a, 0, clk[2], days);
                        sprintf(msg, "Disconnect time #%d:%s\tReconnect time #%d:%s\toffline: %s %s", i - 1, clk[1], i, clk[0], days, clk[2]);
                        Serial.println(msg);
                }
                Serial.println("±±±±± CONNECT ±±±±±");
                for (int i = 0; i < con - 1; i++)
                {
                        char clk[3][20];
                        char days[10];
                        char msg[150];
                        unsigned int a = calc_con_time(i);
                        iot.get_timeStamp(connLOG[i]);
                        sprintf(clk[0], iot.timeStamp);
                        iot.get_timeStamp(disconnLOG[i]);
                        sprintf(clk[1], iot.timeStamp);

                        iot.convert_epoch2clock(a, 0, clk[2], days);
                        sprintf(msg, "Connect time #%d:%s\tDisconnect time #%d:%s\tonline: %s %s", i, clk[0], i, clk[1], days, clk[2]);
                        Serial.println(msg);
                }
        }
}
// ~~~~~ LOW-Level inspections ~~~~~~~~~
void pingServices()
{
        byte retries = 0;
        bool internet_ping = false;
        static unsigned long last_check = 0;

        if (millis() - last_check >= adaptive_ping_val * 1000L)
        {
                if (WiFi.status() == WL_CONNECTED)
                {

                        while (internet_ping == false && retries < 3)
                        {
                                internet_ping = iot.checkInternet("www.google.com", 2);
                                retries++;
                        }
                        retries = 0;
                        while (mqttConnected == false && retries < 3)
                        {
                                mqttConnected = iot.checkInternet("192.168.3.200", 2);
                                retries++;
                        }
                        retries = 0;
                        while (homeAssistantConnected == false && retries < 3)
                        {
                                homeAssistantConnected = iot.checkInternet("192.168.3.199", 2);
                                retries++;
                        }
                }

                last_check = millis();
                checknLOG_internet(internet_ping);
                if (internet_ping == 0)
                {
                        Serial.println("BAD INTERNET");
                }
                if (mqttConnected == 0)
                {
                        Serial.println("BAD MQTT");
                }
                if (homeAssistantConnected == 0)
                {
                        Serial.println("BAD HASS");
                }
                // updateServices_Alerts();
        }
}
void checknLOG_internet(bool &get_ping)
{
        static int same_state_counter = 0;
        static bool lastConStatus = false;

        if (get_ping != lastConStatus) // change state
        {
                if (get_ping == true) /* internet is back on */
                {
                        Serial.print("Reconnect: ");
                        show_clk();
                        updateLOG(connLOG);
                        internetConnected = true;
                        int x = getLOG_entries(connLOG);
                        if (x > 1)
                        { /* not first boot */
                                Serial.print("offline time: ");
                                Serial.print(calc_discon_time(x - 1));
                                Serial.println("[sec]");
                        }
                        else
                        {
                                Serial.print("First boot: ");
                                show_clk();
                        }
                }
                else /* is now disconnected */
                {
                        internetConnected = false;
                        updateLOG(disconnLOG);
                        Serial.print("Disconnect: ");
                        show_clk();
                        printLOG(connLOG, "Disconnect");
                }
                lastConStatus = get_ping;
                same_state_counter = 0;
                adaptive_ping_val = min_ping_interval;
        }
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

                if (get_ping == false)
                {
                        if (getLOG_entries(disconnLOG) == 0) /* booted up with no connection */
                        {
                                updateLOG(disconnLOG);
                        }
                }
        }
}

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
void simulate_disconnects(int errs = 6)
{
        int init_time_drift = errs * 1200; // sec
        int t_ded = init_time_drift / (2 * errs);

        for (int s = 0; s < errs; s++)
        {
                updateLOG(connLOG, now() - init_time_drift - t_ded);
                init_time_drift -= t_ded;
                updateLOG(disconnLOG, now() - init_time_drift - t_ded);
                init_time_drift -= t_ded;
        }
        // updateLOG(connLOG, now() - 200);
        // updateLOG(disconnLOG, now() - 180);

        // updateLOG(connLOG, now() - 175);
        // updateLOG(disconnLOG, now() - 160);

        // updateLOG(connLOG, now() - 140);
        // updateLOG(disconnLOG, now() - 10);

        if (verifyLOG())
        {
                Serial.println("LOG check OK at boot");
        }
        else
        {
                Serial.println("LOG check fail at boot");
        }

        printLOG(connLOG, "CONNECT");
        printLOG(disconnLOG, "DISCONNECT");
}
void setup()
{
        startIOTservices();
        Serial.print("boot: ");
        show_clk();
        simulate_disconnects();
}

void loop()
{
        iot.looper();
        pingServices();
        static unsigned long last = 0;
        if (millis() - last > 30000)
        {
                last = millis();
                run_connection_report();
        }

        delay(100);
}
