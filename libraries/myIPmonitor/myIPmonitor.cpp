#include <myIPmonitor.h>

IPmonitoring::IPmonitoring(char *IP, char *nick)
    : _IP(IP), nick(nick), _conlog_filename(_str_concat(nick, "_connlog.txt")), _ConnectLOG(_conlog_filename)
{
}
IPmonitoring::~IPmonitoring()
{
        delete[] _conlog_filename;
}
void IPmonitoring::start(F_cb ping, F_cb outmsg)
{
        char a[60];
        _ping_cb = ping;
        if (outmsg != NULL)
        {
                _msgOUT = true;
                _msgout_cb = outmsg;
        }
        if (_boot_chk_internet_ntp())
        {
                sprintf(a, "pings %s", _IP);
                _post_msg(a, 1);
                if (!_startFlogs())
                {
                        _post_msg("Start LOG failed!", 1);
                }
        }
        else
        {
                sprintf(a, "%s NOT Connected. Internet/NTP Failure", nick);
                _post_msg(a, 1);
        }
}
void IPmonitoring::loop()
{
        _reset_bootFailure();
        _ping_looper();
        _loopFlogs();
}
bool IPmonitoring::_boot_chk_internet_ntp()
{
        time_t t = time(nullptr);
        struct tm *tm = localtime(&t);

        if (WiFi.status() == WL_CONNECTED && tm->tm_year > 120)
        {
                _needRESET = false;
                return 1;
        }
        else
        {
                return 0;
        }
}
bool IPmonitoring::_ping_client()
{
        const uint8_t retries = 3;
        uint8_t retCounter = 0;
        bool pingState = false;

        while (pingState == false && retCounter < retries) /* Ping retires until success or retries */
        {
                if (WiFi.status() == WL_CONNECTED)
                {
                        pingState = _ping_cb(_IP, 3);
                }
                retCounter++;
        }

        if (pingState != connState || _firstPing == true) /* Change of connection state or first time */
        {
                if (pingState)
                {
                        _reconnect_cb();
                }
                else
                {
                        _disconnect_cb();
                }
                connState = pingState;
                _firstPing = false;
        }
        return connState;
}
void IPmonitoring::_ping_looper()
{
        const uint8_t pingsOK = 10; /* number of pings the low the ping rate */
        if (millis() >= _lastCheck + _adaptive_ping_val * 1000UL)
        {
                _lastCheck = millis();
                if (_ping_client()) /* Ping OK*/
                {
                        if (_pingCounter == pingsOK)
                        {
                                _adaptive_ping_val = MAXPING_TIME;
                                _post_msg("Max_PING", 2);
                                _pingCounter++;
                        }
                        else
                        {
                                _pingCounter++;
                        }
                }
                else /* Ping fails */
                {
                        if (_pingCounter != 0)
                        {
                                _pingCounter = 0;
                                _adaptive_ping_val = MINPING_TIME;
                                _post_msg("Min_PING", 2);
                        }
                }
        }
}
void IPmonitoring::_disconnect_cb()
{
        char a[50];
        _LOGdisconnection();
        currentstateClk = time(nullptr);
        sprintf(a, "%s disconnect [#%d]", nick, ++dCounter);
        _post_msg(a, 0);
}
void IPmonitoring::_reconnect_cb()
{
        time_t t = time(nullptr);
        if ((t - currentstateClk) >= MINPING_TIME && currentstateClk != 0)
        {
                char a[50], b[20];
                _conv_epoch_duration(t, currentstateClk, b);
                sprintf(a, "reconnect [#%d] after [%s]", dCounter, b);
                _post_msg(a, 1);
                _LOGconnection();
                currentstateClk = t;
        }
        else
        {
                _post_msg("connected", 1);
                _LOGconnection();
                currentstateClk = t;
        }
}
void IPmonitoring::_reset_bootFailure()
{
        if (_needRESET)
        {
                if (reset_delay == 0)
                {
                        reset_delay = millis();
                }
                else
                {
                        if (millis() >= 1000UL * RESET_BOOT_ERR + reset_delay)
                        {
                                _post_msg("No internet on Boot", 1);
                                delay(500);
#if isESP8266
                                ESP.reset();
#elif isESP32
                                ESP.restart();
#endif
                                delay(1000);
                        }
                }
        }
}

// ~~~~~~~~~~ Calculate Conn/Disconn ~~~~~~~
void IPmonitoring::getStatus(int h)
{
        uint8_t return_reason1;
        uint8_t return_reason2;
        time_t return_logTime1;
        time_t return_logTime2;
        const unsigned long hrs2sec = 60 * 60 * h;

        char a[100];
        char connect_duration[20];
        char disco_duration[20];
        unsigned int cum_dTime = 0;
        unsigned int cum_cTime = 0;
        unsigned int total_c = 0;
        unsigned int total_disc = 0;
        unsigned int total_boots = 0;

        const unsigned int total_records = _ConnectLOG.getnumlines();
        for (int i = 0; i < total_records - 1; i++)
        {
                /* return_logTime2 is log entry after return_logTime1 */
                time_t t = time(nullptr);
                _readLOG(_ConnectLOG, i, return_logTime1, return_reason1);

                if (t - return_logTime1 <= hrs2sec) /* Meets time interval critetia */
                {
                        _readLOG(_ConnectLOG, i + 1, return_logTime2, return_reason2);

                        if (return_reason1 == 0 && return_reason2 != 0) /* Disconnect record */
                        {
                                cum_dTime += return_logTime2 - return_logTime1;
                                total_disc++;
                        }
                        else if (return_reason1 == 1 && return_reason2 != 1) /* Reconnect record */
                        {
                                cum_cTime += return_logTime2 - return_logTime1;
                                total_c++;
                        }
                        else if (return_reason1 == 2) /* Reboot Record */
                        {
                                cum_cTime += return_logTime2 - return_logTime1;
                                total_boots++;
                        }
                }
        }

        _readLOG(_ConnectLOG, total_records - 1, return_logTime1, return_reason1); /* last record and present state */
        time_t t = time(nullptr);
        if (return_reason1 == 2)
        {
                cum_cTime += t - return_logTime1;
                total_boots++;
        }
        else if (return_reason1 == 1)
        {
                cum_cTime += t - return_logTime1;
                total_c++;
        }
        else
        {
                cum_dTime += t - return_logTime1;
                total_disc++;
        }

        _conv_epoch_duration(cum_cTime, 0, connect_duration);
        _conv_epoch_duration(cum_dTime, 0, disco_duration);
        sprintf(a, "**Records: Total:[%d]; Disconnects:[%d]; Reconnects:[%d]; Reboots:[%d]", total_records, total_disc, total_c, total_boots);
        _post_msg(a, 0);
        sprintf(a, "**Durations: Disconnected:[%s]; Connected:[%s]", disco_duration, connect_duration);
        _post_msg(a, 0);
}
void IPmonitoring::_post_msg(char *inmsg, uint8_t msg_type)
{
        char msg[150];
        char Clk[20];
        time_t t = time(nullptr);

        _conv_epoch(t, Clk);
        sprintf(msg, "[%s] [%s] %s", Clk, nick, inmsg);
        if (_msgOUT)
        {
                sprintf(msg, "[%s] %s", nick, inmsg);
                _msgout_cb(msg, msg_type);
                Serial.println(msg);
        }
}

// ~~~~~~~~~~FlashLOGS~~~~~~~~~~~
void IPmonitoring::deleteLOG()
{
        _ConnectLOG.delog();
}
void IPmonitoring::printFlog(int i)
{
        if (i == 0)
        {
                uint8_t return_reason;
                time_t return_logTime;
                char msg[80];
                char clock[25];
                const char *logTypes[] = {"Disconnect", "Reconnect", "Boot"};
                int x = _ConnectLOG.getnumlines();
                Serial.print("nick: ");
                Serial.println(nick);
                Serial.print("log_lines: ");
                Serial.println(x);

                for (int a = 0; a < x; a++)
                {
                        _readLOG(_ConnectLOG, a, return_logTime, return_reason);
                        _conv_epoch(return_logTime, clock);
                        sprintf(msg, "[#%03d] [%s] [%s]", a, clock, logTypes[return_reason]);
                        _post_msg(msg, 2);
                        Serial.println(msg);
                }
        }
}
void IPmonitoring::enter_fake_LOGentry(time_t t, uint8_t reason)
{
        char c[20];
        sprintf(c, "%d,%d", t, reason);
        _ConnectLOG.write(c, true);
}
bool IPmonitoring::_startFlogs()
{
        return _ConnectLOG.start(LOG_ENTRIES);
}
void IPmonitoring::_loopFlogs()
{
        _ConnectLOG.looper();
}
void IPmonitoring::_LOGconnection()
{
        time_t t = time(nullptr);
        if (currentstateClk != 0)
        {
                _writeLOG(_ConnectLOG, 1, t, true); /* 1==reconnect */
        }
        else
        {
                _writeLOG(_ConnectLOG, 2, t, true); /* 2== boot & Pןing OK*/
        }
}
void IPmonitoring::_LOGdisconnection()
{
        time_t t = time(nullptr);
        _writeLOG(_ConnectLOG, 0, t, true);
}
void IPmonitoring::_readLOG(flashLOG &LOG, int numLine, time_t &retTime, uint8_t &retType)
{
        char a[20];
        if (LOG.readline(numLine, a))
        {
                _inline_read(a);
                retTime = atoi(_inline_param[0]); // Clock
                retType = atoi(_inline_param[1]); // RType
        }
        else
        {
                Serial.println("BAd read");
        }
}
void IPmonitoring::_writeLOG(flashLOG &LOG, uint8_t Reason, time_t value, bool writenow)
{
        /* Reason can be: 0: disconnect, 1: Reconnect, 2:Boot */
        char msg[20];

        time_t t = time(nullptr);
        struct tm *tm = localtime(&t);

        if (tm->tm_year < 120)
        {
                _post_msg("NTP is not set - not entering logs entries", 1);
        }
        else
        {
                sprintf(msg, "%d,%d", (unsigned long)value, Reason);
                LOG.write(msg, writenow);
        }
}

// ~~~~~~~~~ Utils ~~~~~~~~~~
void IPmonitoring::_conv_epoch(time_t &t, char *retDate)
{
        struct tm *tm = localtime(&t);
        sprintf(retDate, "%02d-%02d-%02d %02d:%02d:%02d", 1900 + tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}
void IPmonitoring::_conv_epoch_duration(long t1, long t2, char *clk)
{
        uint8_t days = 0;
        uint8_t hours = 0;
        uint8_t minutes = 0;
        uint8_t seconds = 0;

        int sec2minutes = 60;
        int sec2hours = (sec2minutes * 60);
        int sec2days = (sec2hours * 24);
        int sec2years = (sec2days * 365);

        long time_delta = t1 - t2;

        days = (int)(time_delta / sec2days);
        hours = (int)((time_delta - days * sec2days) / sec2hours);
        minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

        if (days != 0)
        {
                sprintf(clk, "%dd %02d:%02d:%02d", days, hours, minutes, seconds);
        }
        else
        {
                sprintf(clk, "%02d:%02d:%02d", hours, minutes, seconds);
        }
}
char *IPmonitoring::_str_concat(const char *a, const char *b)
{
        char *s = new char[strlen(a) + strlen(b) + 1 + 1];
        sprintf(s, "/%s%s", a, b);
        return s;
}
uint8_t IPmonitoring::_inline_read(char *inputstr)
{
        char *pch;
        uint8_t i = 0;
        const uint8_t MAX_P = 3;

        pch = strtok(inputstr, " ,.-");
        while (pch != NULL && i < MAX_P)
        {
                sprintf(_inline_param[i], "%s", pch);
                pch = strtok(NULL, " ,.-");
                i++;
        }
        return i;
}