#include <Arduino.h>
#include "myIOT_settings.h"
// #include "OLEDdisplay.h"

// ********** Sketch Services  ***********
#define VER "WEMOS_4.0"
// #define USE_HASS_SERVICE false
// #define USE_MQTT_SERVICE true
// #define USE_INTERNET_SERVICE true
// #define USE_METRICS false
// #define USE_DISPLAY false
// #define USE_SPIFFS_LOG false
// #define MQTT_SERVER_IP "192.168.2.101"
#define MQTT_SERVER_IP "192.168.3.200"
#define HASS_IP "192.168.3.199"
#define INTERNET_IP "www.google.com"
// #define ROUTER_IP "192.168.2.1"
#define ROUTER_IP "192.168.3.1"

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
        unsigned long currentstateClk = 0;
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
        flashLOG _connectionLOG;
        flashLOG _disconnectionLOG;

        unsigned long _connLOG[LOG_ENTRIES];
        unsigned long _disconnLOG[LOG_ENTRIES];

public:
        IPmonitoring(char *IP, char *nick, char *filename1 = "/conlog.txt", char *filename2 = "/disconlog.txt")
            : _connectionLOG(filename1), _disconnectionLOG(filename2)
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
                        sprintf(a, "%s pings %s", _nick, _IP);
                        _post_msg(a);

                        // _startFlogs();
                        // _post_msg("LOGs started")
                }
                else
                {
                        sprintf(a, "%s NOT Connected. Internet/NTP Failure", _nick);
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
                Serial.print(" ");
                Serial.print(_nick);
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
                Serial.print(" ");
                Serial.print(_nick);
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
                _updateLOG(_connLOG);
                printLOG(_connLOG, "ConnectLOG");
        }
        void _LOGdisconnection()
        {
                _updateLOG(_disconnLOG);
                printLOG(_disconnLOG, "DisconnectLOG");
        }
        void _disco_service()
        {
                // if (dClk == 0)
                // {
                char a[50];
                // dClk = now();
                currentstateClk = now();
                dCounter++;
                _LOGdisconnection();
                sprintf(a, "%s disconnect [#%d]", _nick, dCounter);
                _post_msg(a);
                // }
        }
        void _reco_service()
        {
                if (currentstateClk != 0)
                {
                        char a[50];
                        char b[20];
                        char c[10];
                        iot.convert_epoch2clock(now(), currentstateClk, b, c);
                        sprintf(a, "%s reconnect [#%d] after [%s %s]", _nick, dCounter, c, b);

                        currentstateClk = now();
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
                _connectionLOG.start(ENTRY_LENGTH, LOG_ENTRIES);
                _disconnectionLOG.start(ENTRY_LENGTH, LOG_ENTRIES);
        }
        void _check_log_onBoot()
        {
                time_t t = now();
                // After reboot when connected
                if (_connectionLOG.getnumlines() == _disconnectionLOG.getnumlines())
                {
                        _writeFlog(_disconnectionLOG);
                }
                // After reboot when non-connected
                else if (_disconnectionLOG.getnumlines() == _connectionLOG.getnumlines() + 1)
                {
                        _writeFlog(_connectionLOG);
                }
                else
                {
                        char a[50];
                        sprintf(a, "Error: Connect Etries [%d] Disconnect entreis [%d]", _connectionLOG.getnumlines(), _disconnectionLOG.getnumlines());
                        iot.pub_log(a);
                }

                if (year(t) == 1970)
                {
                        iot.pub_log("NTP is not set. Can affect on log entries");
                }
        }
        void _startFlogs()
        {
                _flog_init();
                // _check_log_onBoot()
        }
        void _loopFlogs()
        {
                _disconnectionLOG.looper();
                _connectionLOG.looper();
        }
        // ~~~~~~~~~END FlashLOGS

        bool _validateLOGS()
        {
                int c = getLOG_ENTRIES(_connLOG);
                int d = getLOG_ENTRIES(_disconnLOG);
                bool logerr = true;

                /* Check LOGS are sorted correctly by time stamp */
                for (int i = 0; i < c - 1; i++)
                {
                        if (_connLOG[i] > _connLOG[i + 1])
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
                        if (_disconnLOG[i] > _disconnLOG[i + 1])
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
                                        if (_connLOG[i] > _disconnLOG[i])
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
                int c = getLOG_ENTRIES(_connLOG);
                int d = getLOG_ENTRIES(_disconnLOG);

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
                        _updateLOG(_connLOG, now() - init_time_drift - t_ded);
                        init_time_drift -= t_ded;
                        _updateLOG(_disconnLOG, now() - init_time_drift - t_ded);
                        init_time_drift -= t_ded;
                }
                // _updateLOG(_connLOG, now() - 200);
                // _updateLOG(_disconnLOG, now() - 180);

                // _updateLOG(_connLOG, now() - 175);
                // _updateLOG(_disconnLOG, now() - 160);

                // _updateLOG(_connLOG, now() - 140);
                // _updateLOG(_disconnLOG, now() - 10);

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
