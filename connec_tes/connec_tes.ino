#include <Arduino.h>
#include "myIOT_settings.h"
#include "OLEDdisplay.h"

// ********** Sketch Services  ***********
#define VER "WEMOS_3.1"

bool mqttConnected = false;
bool internetConnected = false;
bool homeAssistantConnected = false;

const byte log_entries = 50;
unsigned long connLOG[log_entries];
unsigned long disconnLOG[log_entries];

const byte min_ping_interval = 10; //seconds
const byte UPDATE_REPORT_PERIOD = 30;
byte adaptive_ping_val = min_ping_interval;

const byte NUM_PERIODS = 4;
const byte PERIOD_0 = 1;
const byte PERIOD_1 = 12;
const byte PERIOD_2 = 24;
const byte PERIOD_3 = 24 * 7;
const float check_times[NUM_PERIODS] = {PERIOD_0, PERIOD_1, PERIOD_2, PERIOD_3};

int totDisconnects = 0;
unsigned long bootclk = 0;
unsigned long totCon_time = 0;
unsigned long period_disc_cumTime[NUM_PERIODS];
int period_disconnects[NUM_PERIODS];
int alertLevel[NUM_PERIODS];

int buttonPin = D3;

void show_clk(time_t t = now())
{
        char clk[20];
        char days[5];
        char msg[50];
        iot.get_timeStamp(t);
        sprintf(msg, "%d, clock:%s", t, iot.timeStamp);
        Serial.println(msg);
}

// ±±±±±±±±±± LOG functions ±±±±±±±±±±±±±±
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

// ±±±±±±±±±±± Connect + Disconnect calcs ±±±±±±±±±±±±±
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
void calc_connection(unsigned long &cum_disconTime, int &disconnects_counter, float timePeriod = 24)
{
        int con = getLOG_entries(connLOG);
        cum_disconTime = 0;
        disconnects_counter = 0;
        unsigned long crit = now() - int(timePeriod * 3600UL);
        for (int i = 1; i < con; i++)
        {
                if (disconnLOG[i - 1] > crit)
                {
                        cum_disconTime += calc_discon_time(i); /* Summing offline times */
                        disconnects_counter++;
                }
        }
}
void run_connection_report(bool show_results = true)
{
        char msg[40];
        char clk[20];
        char days[10];
        const byte ERR_COEFF = 20; //sec

        int disconnect_fail_criteria[][4] = {{PERIOD_0, 2},
                                             {PERIOD_1, 1.5 * PERIOD_1},
                                             {PERIOD_2, 1.5 * PERIOD_2},
                                             {PERIOD_3, 1.5 * PERIOD_3}};
        int disconTime_fail_criteria[][4] = {{ERR_COEFF * PERIOD_0, 1.5 * ERR_COEFF * PERIOD_0},
                                             {ERR_COEFF * PERIOD_1, 1.5 * ERR_COEFF * PERIOD_1},
                                             {ERR_COEFF * PERIOD_2, 1.5 * ERR_COEFF * PERIOD_2},
                                             {ERR_COEFF * PERIOD_3, 1.5 * ERR_COEFF * PERIOD_3}};
        if (show_results)
        {
                display_services_status();
                display_totals();
        }

        for (int i = 0; i < NUM_PERIODS; i++)
        {
                calc_connection(period_disc_cumTime[i], period_disconnects[i], check_times[i]);

                if (period_disconnects[i] >= disconnect_fail_criteria[i][1] || period_disc_cumTime[i] >= disconTime_fail_criteria[i][1])
                {
                        alertLevel[i] = 2;
                }
                else if (period_disconnects[i] >= disconnect_fail_criteria[i][0] && period_disconnects[i] < disconnect_fail_criteria[i][1] || period_disc_cumTime[i] >= disconTime_fail_criteria[i][0 && period_disc_cumTime[i] < disconTime_fail_criteria[i][1]])
                {
                        alertLevel[i] = 1;
                }
                else if (period_disc_cumTime[i] < disconTime_fail_criteria[i][0] || period_disc_cumTime[i] < disconTime_fail_criteria[i][0])
                {
                        alertLevel[i] = 0;
                }
                else
                {
                        Serial.println("LEVEL_ERR");
                }
                if (show_results)
                {
                        Serial.print("\n~~~~~~~~~~~~~~~");
                        Serial.print(check_times[i]);
                        Serial.print("_hrs");
                        Serial.println("~~~~~~~~~~~~~~~");

                        iot.convert_epoch2clock(period_disc_cumTime[i], 0, clk, days);
                        sprintf(msg, "Disonnected in %.2f_hrs:\t%s %s", check_times[i], days, clk);
                        Serial.println(msg);

                        sprintf(msg, "Disconnects in %.2f_hrs:\t%d", check_times[i], period_disconnects[i]);
                        Serial.println(msg);

                        sprintf(msg, "alertLevel in %.2f_hrs:\t\t%d", check_times[i], alertLevel[i]);
                        Serial.println(msg);
                }
        }
}

// ±±±±±±±±±±±±± Display results on Serial ±±±±±±±±±±±±±
void display_services_status()
{
        char msg[50];
        Serial.println("\n~~~~~~ Services Connection Status ~~~~~");
        sprintf(msg, ">> Internet :\t\t\t[%s]", internetConnected ? "Connected" : "Disconnected");
        Serial.println(msg);
        sprintf(msg, ">> MQTT :\t\t\t[%s]", mqttConnected ? "Connected" : "Disconnected");
        Serial.println(msg);
        sprintf(msg, ">> HomeAssistant :\t\t[%s]", homeAssistantConnected ? "Connected" : "Disconnected");
        Serial.println(msg);
}
void display_totals()
{
        char msg[40];
        char clk[20];
        char days[10];

        calc_total_time(totCon_time);
        totDisconnects = getLOG_entries(disconnLOG);

        Serial.println("\n~~~~~~~~~~~~~~~ Totals ~~~~~~~~~~~~~~~");
        iot.convert_epoch2clock(totCon_time, 0, clk, days);
        sprintf(msg, "Total monitoring time:\t\t%s %s", days, clk);
        Serial.println(msg);
        sprintf(msg, "Total disonnects:\t\t%d", totDisconnects);
        Serial.println(msg);
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
                        Serial.println(">> BAD INTERNET");
                }
                if (mqttConnected == 0)
                {
                        Serial.println(">> BAD MQTT");
                }
                if (homeAssistantConnected == 0)
                {
                        Serial.println(">> BAD HASS");
                }
        }
}
void update_ConnectionReport(byte check_int = 30)
{
        static unsigned long last = 0;
        if (millis() - last > check_int * 1000UL)
        {
                last = millis();
                run_connection_report();
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
                        // Serial.print("\n>>Reconnect: ");
                        // show_clk();
                        updateLOG(connLOG);
                        internetConnected = true;
                        int x = getLOG_entries(connLOG);
                        if (x > 1)
                        { /* not first boot */
                                Serial.print("\n>>Offline time: ");
                                Serial.print(calc_discon_time(x - 1));
                                Serial.println("[sec]");
                        }
                        // else
                        // {
                        //         Serial.print("\n>>First boot: ");
                        //         show_clk();
                        // }
                }
                else /* is now disconnected */
                {
                        internetConnected = false;
                        updateLOG(disconnLOG);
                        Serial.print("\n>>Disconnect: ");
                        show_clk();
                }
                lastConStatus = get_ping;
                same_state_counter = 0;
                adaptive_ping_val = min_ping_interval;
                printLOG(connLOG, "CONNECT");
                printLOG(disconnLOG, "DISCONNECT");
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

        // onBoot_clk();
        if (verifyLOG())
        {
                Serial.println("LOG check OK at boot");
        }
        else
        {
                Serial.println("LOG check fail at boot");
        }
}

void setup()
{
        pinMode(buttonPin, INPUT_PULLUP);
        startIOTservices();
        bootclk = now();
        // simulate_disconnects();
        OLED.start();
}
void loop()
{
        iot.looper();
        pingServices();
        update_ConnectionReport(UPDATE_REPORT_PERIOD);
        displays_looper();
        delay(100);
}
