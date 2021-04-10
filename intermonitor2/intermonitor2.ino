#include <Arduino.h>
#include "myIOT_settings.h"
// #include "OLEDdisplay.h"

// ********** Sketch Services  ***********
#define VER "WEMOS_4.0"
#define USE_HASS_SERVICE false
#define USE_MQTT_SERVICE true
#define USE_INTERNET_SERVICE true
#define USE_METRICS false
#define USE_DISPLAY false
#define USE_SPIFFS_LOG false
// #define MQTT_SERVER_IP "192.168.2.101"
#define MQTT_SERVER_IP "192.168.3.200"
#define HASS_IP "192.168.3.199"
#define INTERNET_IP "www.google.com"
// #define ROUTER_IP "192.168.2.1"
#define ROUTER_IP "192.168.3.1"

#define MAXPING_TIME 20    /* Max time between pings, sec */
#define MINPING_TIME 10    /* Min time between pings, sec */
#define TIME_TO_MAXPING 30 /* After this time period ping will be every MAXPING_TIME sec */
#define RESET_BOOT_ERR 2   /* Time to wait until reset due to NO-internet or NO-NTP failure */

struct serviceMonitor
{
        unsigned long startClock;
        unsigned long bootClk;
        unsigned long dClk;
        unsigned int dCounter;
        unsigned int dDuration;
        bool isConnected;
        char *IP;
        char *nick;
};
#if USE_MQTT_SERVICE
serviceMonitor MQTT_monitor{0, 0, 0, 0, 0, false, MQTT_SERVER_IP, "MQTTbroker"};
#endif
#if USE_INTERNET_SERVICE
serviceMonitor internet_monitor{0, 0, 0, 0, 0, false, INTERNET_IP, "Internet"};
#endif
#if USE_HASS_SERVICE
serviceMonitor HASS_monitor{0, 0, 0, 0, 0, false, HASS_IP, "HomeAssistant"};
#endif
serviceMonitor WiFi_monitor{0, 0, 0, 0, 0, false, ROUTER_IP, "WiFi"};

#include "log.h"

byte adaptive_ping_val = MINPING_TIME;
bool needRESET = true;

void post_msg(char *inmsg, char *inmsg2 = "")
{
        String msg;
        iot.get_timeStamp();
        msg = "[" + String(iot.timeStamp) + "] " + String(inmsg) + String(inmsg2);
        Serial.println(msg);
}
void LOGconnection()
{
        updateLOG(connLOG);
        printLOG(connLOG, "ConnectLOG");
}
void LOGdisconnection()
{
        updateLOG(disconnLOG);
        printLOG(disconnLOG, "DisconnectLOG");
}
void disco_service(serviceMonitor &service)
{
        if (service.dClk == 0)
        {
                char a[50];
                service.dClk = now();
                service.dCounter++;
                LOGdisconnection();
                sprintf(a, "%s disconnect [#%d]", service.nick, service.dCounter);
                post_msg(a);
        }
}
void reco_service(serviceMonitor &service)
{
        if (service.dClk != 0)
        {
                char a[50];
                char b[20];
                char c[10];
                iot.convert_epoch2clock(now(), service.dClk, b, c);
                sprintf(a, "%s reconnect [#%d] after [%s %s]", service.nick, service.dCounter, c, b);

                service.dClk = 0;
                service.dDuration = 0;
                post_msg(a);
        }
        else
        {
                post_msg(service.nick, " connected");
        }
        LOGconnection();
}
bool ping_client(serviceMonitor &service, byte retries = 3)
{
        bool pingOK = false;
        byte retCounter = 0;

        while (pingOK == false && retCounter < retries)
        {
                if (WiFi.status() == WL_CONNECTED)
                {
                        pingOK = iot.checkInternet(service.IP, 2);
                }
                else if (WiFi.status() != WL_CONNECTED && strcmp(service.nick, WiFi_monitor.nick) != 0) /* No WiFi - but check other services */
                {
                        Serial.print("No WiFi - can't ping service ");
                        Serial.println(service.nick);
                }
                else /* No WiFi */
                {
                        service.isConnected = false;
                }
                retCounter++;
        }

        if (pingOK != service.isConnected)
        {
                if (pingOK == true)
                {
                        reco_service(service);
                }
                else
                {
                        disco_service(service);
                }
                service.isConnected = pingOK;
        }

        return service.isConnected;
}
void check_all_services()
{
        static unsigned long lastCheck = 0;
        static unsigned long all_good_clk = 0;
        if (millis() >= lastCheck + adaptive_ping_val * 1000L)
        {
                bool test = true;
                bool a = false;
                lastCheck = millis();
#if USE_INTERNET_SERVICE
                a = ping_client(internet_monitor);
                test = test && a;
                if (a == false)
                {
                        post_msg(internet_monitor.nick, " Fail");
                }
#endif
#if USE_HASS_SERVICE
                a = ping_client(HASS_monitor);
                test = test && a;
                if (a == false)
                {
                        post_msg(HASS_monitor.nick, " Fail");
                }
#endif
#if USE_MQTT_SERVICE
                a = ping_client(MQTT_monitor);
                test = test && a;
                if (a == false)
                {
                        post_msg(MQTT_monitor.nick, " Fail");
                }
#endif
                a = ping_client(WiFi_monitor);
                test = test && a;
                if (a == false)
                {
                        post_msg(WiFi_monitor.nick, " Fail");
                }

                if (test)
                {
                        if (all_good_clk == 0)
                        {
                                all_good_clk = now();
                                post_msg("All services-\"Ping OK\"");
                        }
                }
                else
                {
                        all_good_clk = 0;
                        adaptive_ping_val = MINPING_TIME;
                        post_msg("Return to minPing");
                }
                if (now() - all_good_clk > TIME_TO_MAXPING && adaptive_ping_val != MAXPING_TIME)
                {
                        adaptive_ping_val = MAXPING_TIME;
                        post_msg("Switch to maxPing");
                }
        }
}

bool verify_internet_ok()
{
        if (WiFi.status() == WL_CONNECTED && iot.NTP_OK)
        {
                needRESET = false;
                return 1;
        }
        else
        {
                return 0;
        }
}
void start_services()
{
        if (verify_internet_ok())
        {
#if USE_MQTT_SERVICE
                MQTT_monitor.bootClk = now();
                post_msg(MQTT_monitor.nick, MQTT_monitor.IP);
#endif
#if USE_INTERNET_SERVICE
                internet_monitor.bootClk = now();
                post_msg(internet_monitor.nick, internet_monitor.IP);
#endif
#if USE_HASS_SERVICE
                HASS_monitor.bootClk = now();
                post_msg(HASS_monitor.nick, HASS_monitor.nick);
#endif
                WiFi_monitor.bootClk = now();
                post_msg(WiFi_monitor.nick, WiFi_monitor.IP);

#if USE_SPIFFS_LOG
                startFlogs();
                post_msg("FS LOG started")
#endif
        }
        else
        {
                post_msg("Internet/ NTP - Boot Fail");
        }
}
void reset_bootFailure()
{
        static long reset_delay = 0;
        if (needRESET)
        {
                if (reset_delay == 0)
                {
                        reset_delay = millis();
                }
                else
                {
                        if (millis() >= 1000L * RESET_BOOT_ERR + reset_delay)
                        {
                                iot.sendReset("No internet on Boot");
                        }
                }
        }
}

class IPmonitoring
{
#define MAXPING_TIME 20    /* Max time between pings, sec */
#define MINPING_TIME 10    /* Min time between pings, sec */
#define TIME_TO_MAXPING 30 /* After this time period ping will be every MAXPING_TIME sec */
#define RESET_BOOT_ERR 2   /* Time to wait until reset due to NO-internet or NO-NTP failure */

#define ENTRY_LENGTH 200
#define LOG_ENTRIES 50

public:
        unsigned long bootClk = 0;
        unsigned long dClk = 0;
        unsigned long cstateClk = 0;
        unsigned int dCounter = 0;
        unsigned int dDuration = 0;
        bool isConnected = false;

private:
        char *_IP;
        char *_nick;
        byte _adaptive_ping_val = MINPING_TIME;
        byte _pingCounter = 0;
        bool _needRESET = true;
        unsigned long _lastCheck = 0;
        flashLOG connectionLOG;
        flashLOG disconnectionLOG;

        unsigned long connLOG[LOG_ENTRIES];
        unsigned long disconnLOG[LOG_ENTRIES];

public:
        IPmonitoring(char *IP, char *nick, char *filename1 = "/conlog.txt", char *filename2 = "/disconlog.txt")
            : connectionLOG(filename1), disconnectionLOG(filename2)
        {
                _IP = IP;
                _nick = nick;
        }
        void start()
        {
                char a[80];

                if (_verify_internet_ok())
                {
                        bootClk = now();
                        sprintf(a, "%s Pings %s", _nick, _IP);
                        _post_msg(a);

                        // _startFlogs();
                        // _post_msg("LOGs started")
                }
                else
                {
                        sprintf(a, "%s NOT Connected. Internet/NTP Failure", WiFi_monitor.nick);
                        _post_msg(a);
                }
        }
        void loop()
        {
                _reset_bootFailure();
                _ping_looper();
                // _loopFlogs()
        }
        void printLOG(unsigned long LOG[], char *topic)
        {
                char msg[40];
                char clk[20];
                char days[10];

                Serial.print(" \n*** Start ");
                Serial.print(topic);
                Serial.println(" *** ");
                for (int i = 0; i < getLOG_ENTRIES(LOG); i++)
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
        int getLOG_ENTRIES(unsigned long LOG[])
        {
                int i = 0;
                while (i < LOG_ENTRIES)
                {
                        if (LOG[i] == 0)
                        {
                                break;
                        }
                        i++;
                }
                return i;
        }

private:
        void _LOGconnection()
        {
                // updateLOG(connLOG);
                // printLOG(connLOG, "ConnectLOG");
        }
        void _LOGdisconnection()
        {
                // updateLOG(disconnLOG);
                // printLOG(disconnLOG, "DisconnectLOG");
        }
        void _disco_service()
        {
                if (dClk == 0)
                {
                        char a[50];
                        dClk = now();
                        dCounter++;
                        _LOGdisconnection();
                        sprintf(a, "%s disconnect [#%d]", _nick, dCounter);
                        _post_msg(a);
                }
        }
        void _reco_service()
        {
                if (dClk != 0)
                {
                        char a[50];
                        char b[20];
                        char c[10];
                        iot.convert_epoch2clock(now(), dClk, b, c);
                        sprintf(a, "%s reconnect [#%d] after [%s %s]", _nick, dCounter, c, b);

                        dClk = 0;
                        dDuration = 0;
                        _post_msg(a);
                }
                else
                {
                        _post_msg(_nick, " connected");
                }
                _LOGconnection();
        }

        bool _ping_client()
        {
                byte retries = 3;
                bool pingOK = false;
                byte retCounter = 0;

                while (pingOK == false && retCounter < retries)
                {
                        if (WiFi.status() == WL_CONNECTED)
                        {
                                pingOK = iot.checkInternet(_IP, 2);
                        }
                        retCounter++;
                }

                if (pingOK != isConnected)
                {
                        if (pingOK == true)
                        {
                                _reco_service();
                        }
                        else
                        {
                                _disco_service();
                        }
                        isConnected = pingOK;
                }

                return isConnected;
        }
        void _ping_looper()
        {
                const byte pingsOK = 3; /* number of pings the low the ping rate */
                if (millis() >= _lastCheck + _adaptive_ping_val * 1000L)
                {
                        _lastCheck = millis();
                        if (_ping_client())
                        {
                                if (_pingCounter <= pingsOK)
                                {
                                        _pingCounter++;
                                }
                        }
                        else
                        {
                                _pingCounter = 0;
                                _adaptive_ping_val = MINPING_TIME;
                                _post_msg("Min_PING");
                        }
                        if (_pingCounter == pingsOK)
                        {
                                _adaptive_ping_val = MAXPING_TIME;
                                _post_msg("Max_PING");
                        }
                }
        }

        bool _verify_internet_ok()
        {
                if (WiFi.status() == WL_CONNECTED && iot.NTP_OK)
                {
                        _needRESET = false;
                        return 1;
                }
                else
                {
                        return 0;
                }
        }
        void _post_msg(char *inmsg, char *inmsg2 = "")
        {
                String msg;
                iot.get_timeStamp();
                msg = "[" + String(iot.timeStamp) + "] " + String(inmsg) + String(inmsg2);
                Serial.println(msg);
        }
        void _reset_bootFailure()
        {
                static long reset_delay = 0;
                if (_needRESET)
                {
                        if (reset_delay == 0)
                        {
                                reset_delay = millis();
                        }
                        else
                        {
                                if (millis() >= 1000L * RESET_BOOT_ERR + reset_delay)
                                {
                                        iot.sendReset("No internet on Boot");
                                }
                        }
                }
        }

        // ~~~~~~~~~~FlashLOGS
        time_t _readFlog(flashLOG &LOG, int numLine)
        {
                char a[14];
                time_t ret = 0;

                if (LOG.readline(numLine, a))
                {
                        ret = atoi(a);
                }
                return ret;
        }
        void _writeFlog(flashLOG &LOG, time_t value = now(), bool writenow = false)
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
        void _flog_init()
        {
                connectionLOG.start(ENTRY_LENGTH, LOG_ENTRIES);
                disconnectionLOG.start(ENTRY_LENGTH, LOG_ENTRIES);
        }
        void _check_log_onBoot()
        {
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
        void _startFlogs()
        {
                flog_init();
                // check_log_onBoot()
        }
        void _loopFlogs()
        {
                disconnectionLOG.looper();
                connectionLOG.looper();
        }
        // ~~~~~~~~~END FlashLOGS

        bool _validateLOGS()
        {
                int c = getLOG_ENTRIES(connLOG);
                int d = getLOG_ENTRIES(disconnLOG);
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
                if (WiFi.status() == WL_CONNECTED)
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
        void _fixLOG()
        {
                int c = getLOG_ENTRIES(connLOG);
                int d = getLOG_ENTRIES(disconnLOG);

                if (WiFi.status() == WL_CONNECTED)
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
        int _driftLOG(unsigned long LOG[])
        {
                int i = getLOG_ENTRIES(LOG);
                if (i >= LOG_ENTRIES - 1)
                {
                        for (int x = 0; x < i; x++)
                        {
                                LOG[x] = LOG[x + 1];
                                Serial.print("drif_log_#");
                                Serial.println(x);
                        }
                }
                return i;
        }
        void _updateLOG(unsigned long LOG[], unsigned long now = now())
        {

                int i = _driftLOG(LOG); /* If log is full- swap is done */
                if (now != 0 && LOG[i - 1] < now)
                {
                        LOG[i] = now;
                }
                else
                {
                        Serial.println("entry error for log. entry ignored");
                }
        }

        void simulate_disconnects(int errs = 6)
        {
                int init_time_drift = errs * 1200; // sec
                int t_ded = init_time_drift / (2 * errs);

                for (int s = 0; s < errs; s++)
                {
                        _updateLOG(connLOG, now() - init_time_drift - t_ded);
                        init_time_drift -= t_ded;
                        _updateLOG(disconnLOG, now() - init_time_drift - t_ded);
                        init_time_drift -= t_ded;
                }
                // _updateLOG(connLOG, now() - 200);
                // _updateLOG(disconnLOG, now() - 180);

                // _updateLOG(connLOG, now() - 175);
                // _updateLOG(disconnLOG, now() - 160);

                // _updateLOG(connLOG, now() - 140);
                // _updateLOG(disconnLOG, now() - 10);

                // onBoot_clk();
                if (_validateLOGS())
                {
                        Serial.println("LOG check OK at boot");
                }
                else
                {
                        Serial.println("LOG check fail at boot");
                }
        }
};

IPmonitoring MQTT_service(MQTT_SERVER_IP, "MQTT_Broker");
IPmonitoring WiFi_service(ROUTER_IP, "Router");
IPmonitoring Internet_service(INTERNET_IP, "Internet");
IPmonitoring HASS_service(HASS_IP, "HomeAssistant");

void setup()
{
        startIOTservices();
        MQTT_service.start();
        WiFi_service.start();
        Internet_service.start();
        HASS_service.start();
        // start_services();
}
void loop()
{
        iot.looper();
        MQTT_service.loop();
        WiFi_service.loop();
        Internet_service.loop();
        HASS_service.loop();
        //         check_all_services();
        //         reset_bootFailure(); /* init reset when boot on no-internet & NTP */
        // #if USE_SPIFFS_LOG
        //         loopFlogs();
        // #endif
        delay(100);
}

// const byte NUM_PERIODS = 4;
// const byte PERIOD_0 = 1;
// const byte PERIOD_1 = 12;
// const byte PERIOD_2 = 24;
// const byte PERIOD_3 = 24 * 7;
// const float check_times[NUM_PERIODS] = {PERIOD_0, PERIOD_1, PERIOD_2, PERIOD_3};

// int totDisconnects = 0;
// unsigned long bootclk = 0;
// unsigned long totCon_time = 0;
// unsigned long period_disc_cumTime[NUM_PERIODS];
// int period_disconnects[NUM_PERIODS];
// int alertLevel[NUM_PERIODS];

// int buttonPin = D3;

// void show_clk(time_t t = now())
// {
//         char clk[20];
//         char days[5];
//         char msg[50];
//         iot.get_timeStamp(t);
//         sprintf(msg, "%d, clock:%s", t, iot.timeStamp);
//         Serial.println(msg);
// }

// // ±±±±±±±±±±± Connect + Disconnect calcs ±±±±±±±±±±±±±
// int calc_discon_time(int x = 0)
// {
//         int c = getLOG_entries(connLOG);
//         int calc = -1;
//         if (x > 0 && x < c)
//         {
//                 if (verifyLOG())
//                 {
//                         calc = connLOG[x] - disconnLOG[x - 1];
//                 }
//                 else
//                 {
//                         Serial.println("log error");
//                         calc = 0;
//                 }
//         }
//         else
//         {
//                 Serial.println("out bound disconn log");
//         }

//         return calc;
// }
// int calc_con_time(int x = 0)
// {
//         int c = getLOG_entries(connLOG);
//         int d = getLOG_entries(disconnLOG);
//         int calc = -1;
//         if (x < d)
//         {
//                 if (verifyLOG())
//                 {
//                         calc = disconnLOG[x] - connLOG[x];
//                 }
//                 else
//                 {
//                         Serial.println("log error");
//                         calc = 0;
//                 }
//         }
//         else if (x == c - 1)
//         {
//                 calc = now() - connLOG[c - 1];
//         }

//         return calc;
// }
// void calc_total_time(unsigned long &total_time)
// {
//         if (verifyLOG)
//         {
//                 total_time = now() - connLOG[0];
//         }
//         else
//         {
//                 total_time = 0;
//                 Serial.println("error total time");
//         }
// }
// void calc_connection(unsigned long &cum_disconTime, int &disconnects_counter, float timePeriod = 24)
// {
//         int con = getLOG_entries(connLOG);
//         cum_disconTime = 0;
//         disconnects_counter = 0;
//         unsigned long crit = now() - int(timePeriod * 3600UL);
//         for (int i = 1; i < con; i++)
//         {
//                 if (disconnLOG[i - 1] > crit)
//                 {
//                         cum_disconTime += calc_discon_time(i); /* Summing offline times */
//                         disconnects_counter++;
//                 }
//         }
// }
// void run_connection_report(bool show_results = true)
// {
//         char msg[40];
//         char clk[20];
//         char days[10];
//         const byte ERR_COEFF = 20; //sec

//         int disconnect_fail_criteria[][4] = {{PERIOD_0, 2},
//                                              {PERIOD_1, 1.5 * PERIOD_1},
//                                              {PERIOD_2, 1.5 * PERIOD_2},
//                                              {PERIOD_3, 1.5 * PERIOD_3}};
//         int disconTime_fail_criteria[][4] = {{ERR_COEFF * PERIOD_0, 1.5 * ERR_COEFF * PERIOD_0},
//                                              {ERR_COEFF * PERIOD_1, 1.5 * ERR_COEFF * PERIOD_1},
//                                              {ERR_COEFF * PERIOD_2, 1.5 * ERR_COEFF * PERIOD_2},
//                                              {ERR_COEFF * PERIOD_3, 1.5 * ERR_COEFF * PERIOD_3}};
//         if (show_results)
//         {
//                 display_services_status();
//                 display_totals();
//         }

//         for (int i = 0; i < NUM_PERIODS; i++)
//         {
//                 calc_connection(period_disc_cumTime[i], period_disconnects[i], check_times[i]);

//                 if (period_disconnects[i] >= disconnect_fail_criteria[i][1] || period_disc_cumTime[i] >= disconTime_fail_criteria[i][1])
//                 {
//                         alertLevel[i] = 2;
//                 }
//                 else if (period_disconnects[i] >= disconnect_fail_criteria[i][0] && period_disconnects[i] < disconnect_fail_criteria[i][1] || period_disc_cumTime[i] >= disconTime_fail_criteria[i][0 && period_disc_cumTime[i] < disconTime_fail_criteria[i][1]])
//                 {
//                         alertLevel[i] = 1;
//                 }
//                 else if (period_disc_cumTime[i] < disconTime_fail_criteria[i][0] || period_disc_cumTime[i] < disconTime_fail_criteria[i][0])
//                 {
//                         alertLevel[i] = 0;
//                 }
//                 else
//                 {
//                         Serial.println("LEVEL_ERR");
//                 }
//                 if (show_results)
//                 {
//                         Serial.print("\n~~~~~~~~~~~~~~~");
//                         Serial.print(check_times[i]);
//                         Serial.print("_hrs");
//                         Serial.println("~~~~~~~~~~~~~~~");

//                         iot.convert_epoch2clock(period_disc_cumTime[i], 0, clk, days);
//                         sprintf(msg, "Disonnected in %.2f_hrs:\t%s %s", check_times[i], days, clk);
//                         Serial.println(msg);

//                         sprintf(msg, "Disconnects in %.2f_hrs:\t%d", check_times[i], period_disconnects[i]);
//                         Serial.println(msg);

//                         sprintf(msg, "alertLevel in %.2f_hrs:\t\t%d", check_times[i], alertLevel[i]);
//                         Serial.println(msg);
//                 }
//         }
// }

// // ±±±±±±±±±±±±± Display results on Serial ±±±±±±±±±±±±±
// void display_services_status()
// {
//         char msg[50];
//         Serial.println("\n~~~~~~ Services Connection Status ~~~~~");
//         sprintf(msg, ">> Internet :\t\t\t[%s]", internetConnected ? "Connected" : "Disconnected");
//         Serial.println(msg);
//         sprintf(msg, ">> MQTT :\t\t\t[%s]", mqttConnected ? "Connected" : "Disconnected");
//         Serial.println(msg);
//         sprintf(msg, ">> HomeAssistant :\t\t[%s]", homeAssistantConnected ? "Connected" : "Disconnected");
//         Serial.println(msg);
// }
// void display_totals()
// {
//         char msg[40];
//         char clk[20];
//         char days[10];

//         calc_total_time(totCon_time);
//         totDisconnects = getLOG_entries(disconnLOG);

//         Serial.println("\n~~~~~~~~~~~~~~~ Totals ~~~~~~~~~~~~~~~");
//         iot.convert_epoch2clock(totCon_time, 0, clk, days);
//         sprintf(msg, "Total monitoring time:\t\t%s %s", days, clk);
//         Serial.println(msg);
//         sprintf(msg, "Total disonnects:\t\t%d", totDisconnects);
//         Serial.println(msg);
// }
// void display_logs()
// {
//         int con = getLOG_entries(connLOG);
//         int discon = getLOG_entries(disconnLOG);

//         if (internetConnected && verifyLOG()) /* nominal: internet connected without log errors*/
//         {
//                 Serial.println("±±±±± DISCONNECT ±±±±±");
//                 for (int i = 1; i <= discon; i++)
//                 {
//                         char clk[3][20];
//                         char days[10];
//                         char msg[150];
//                         unsigned int a = calc_discon_time(i);
//                         iot.get_timeStamp(connLOG[i]);
//                         sprintf(clk[0], iot.timeStamp);
//                         iot.get_timeStamp(disconnLOG[i - 1]);
//                         sprintf(clk[1], iot.timeStamp);

//                         iot.convert_epoch2clock(a, 0, clk[2], days);
//                         sprintf(msg, "Disconnect time #%d:%s\tReconnect time #%d:%s\toffline: %s %s", i - 1, clk[1], i, clk[0], days, clk[2]);
//                         Serial.println(msg);
//                 }
//                 Serial.println("±±±±± CONNECT ±±±±±");
//                 for (int i = 0; i < con - 1; i++)
//                 {
//                         char clk[3][20];
//                         char days[10];
//                         char msg[150];
//                         unsigned int a = calc_con_time(i);
//                         iot.get_timeStamp(connLOG[i]);
//                         sprintf(clk[0], iot.timeStamp);
//                         iot.get_timeStamp(disconnLOG[i]);
//                         sprintf(clk[1], iot.timeStamp);

//                         iot.convert_epoch2clock(a, 0, clk[2], days);
//                         sprintf(msg, "Connect time #%d:%s\tDisconnect time #%d:%s\tonline: %s %s", i, clk[0], i, clk[1], days, clk[2]);
//                         Serial.println(msg);
//                 }
//         }
// }
