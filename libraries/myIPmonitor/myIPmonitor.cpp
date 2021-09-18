#include <myIPmonitor.h>

IPmonitoring::IPmonitoring(char *IP, char *nick)
    : _IP(IP), nick(nick), _conlog_filename(_str_concat(nick, "_connlog.txt")), _conFlog(_conlog_filename)
{
}
IPmonitoring::~IPmonitoring()
{
        delete[] _conlog_filename;
}
void IPmonitoring::start(cb_func ping, cb_func outmsg)
{
        char a[80];
        _ping_cb = ping;
        if (outmsg != NULL)
        {
                _msgOUT = true;
                _msgout_cb = outmsg;
        }
        if (_verify_internet_ok())
        {
                bootClk = time(nullptr);
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
bool IPmonitoring::_verify_internet_ok()
{
        time_t t = time(nullptr);
        struct tm *tm = localtime(&t);

        if (WiFi.status() == WL_CONNECTED && tm->tm_year != 1970)
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
        bool pingOK = false;

        while (pingOK == false && retCounter < retries) /* Ping retires until success or retries */
        {
                if (WiFi.status() == WL_CONNECTED)
                {
                        pingOK = _ping_cb(_IP, 3);
                }
                retCounter++;
        }

        if (pingOK != isConnected || _firstPing == true) /* Change of connection state or first time */
        {
                if (pingOK == true)
                {
                        _reconnect_cb();
                }
                else
                {
                        _disconnect_cb();
                }
                isConnected = pingOK;
                _firstPing = false;
        }
        return isConnected;
}
void IPmonitoring::_ping_looper()
{
        const uint8_t pingsOK = 10; /* number of pings the low the ping rate */
        if (millis() >= _lastCheck + _adaptive_ping_val * 1000UL)
        {
                _lastCheck = millis();
                if (_ping_client()) /* Ping OK*/
                {
                        if (_pingCounter <= pingsOK)
                        {
                                _pingCounter++;
                        }
                }
                else                /* Ping fails */
                {
                        if (_pingCounter != 0)
                        {
                                _pingCounter = 0;
                                _adaptive_ping_val = MINPING_TIME;
                                _post_msg("Min_PING", 1);
                        }
                }

                if (_pingCounter == pingsOK)
                {
                        _adaptive_ping_val = MAXPING_TIME;
                        _post_msg("Max_PING", 1);
                }
        }
}
void IPmonitoring::_disconnect_cb()
{
        char a[50];
        time_t t = time(nullptr);

        currentstateClk = t;
        dCounter++;
        _LOGdisconnection();
        sprintf(a, "%s disconnect [#%d]", nick, dCounter);
        _post_msg(a, 0);
}
void IPmonitoring::_reconnect_cb()
{
        time_t t = time(nullptr);

        if (currentstateClk >= MINPING_TIME)
        {
                char a[50];
                char b[20];

                _conv_epoch_duration(t, currentstateClk, b);
                sprintf(a, "reconnect [#%d] after [%s]", dCounter, b);
                _post_msg(a, 0);
                _LOGconnection();
                currentstateClk = t;
        }
        // if (currentstateClk < MAXPING_TIME && currentstateClk != 0)
        // {
        //         _conFlog.del_last_record();
        //         dCounter--;
        //         _post_msg("Record Deleted");
        // }
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

        const unsigned int total_records = _conFlog.getnumlines();
        for (int i = 0; i < total_records - 1; i++)
        {
                /* return_logTime2 is log entry after return_logTime1 */
                time_t t = time(nullptr);
                _readFlog_2row(_conFlog, i, return_logTime1, return_reason1);
                if (t - return_logTime1 <= hrs2sec) /* Meets time interval critetia */
                {
                        _readFlog_2row(_conFlog, i + 1, return_logTime2, return_reason2);

                        if (return_reason1 == 0 && return_reason2 != 0)               /* Disconnect record */
                        {
                                cum_dTime += return_logTime2 - return_logTime1;
                                total_disc++;
                        }
                        else if (return_reason1 == 1 && return_reason2 != 1)          /* Reconnect record */
                        {
                                cum_cTime += return_logTime2 - return_logTime1;
                                total_c++;
                        }
                        else if (return_reason1 == 2)                                 /* Reboot Record */
                        {
                                cum_cTime += return_logTime2 - return_logTime1;
                                total_boots++;
                        }
                }
        }

        _readFlog_2row(_conFlog, total_records - 1, return_logTime1, return_reason1); /* last record and present state */ 
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
        sprintf(a, "Records: Total/ Disconnects/ Reconnects/ Reboots:[%d/ %d/ %d/ %d]", total_records, total_disc, total_c, total_boots);
        _post_msg(a, 0);
        sprintf(a, "Durations: Disconnected/Connected:[%s/ %s]", disco_duration, connect_duration);
        _post_msg(a, 0);
}
void IPmonitoring::_post_msg(char *inmsg, uint8_t msg_type)
{
        char msg[150];
        char Clk[20];
        time_t t = time(nullptr);

        _conv_epoch(t, Clk);
        sprintf(msg, "[%s] [%s] %s", Clk, nick, inmsg);
        Serial.println(msg);
        if (_msgOUT)
        {
                sprintf(msg, "[%s] %s", nick, inmsg);
                Serial.println(msg_type);
                _msgout_cb(msg, msg_type);
        }
}

// ~~~~~~~~~~FlashLOGS~~~~~~~~~~~
void IPmonitoring::deleteLOG()
{
        _conFlog.delog();
}
void IPmonitoring::printFlog(int i)
{
        if (i == NULL)
        {
                uint8_t return_reason;
                time_t return_logTime;
                char msg[80];
                char clock[25];
                const char *logTypes[] = {"Disconnect", "Reconnect", "Boot"};
                int x = _conFlog.getnumlines();

                for (int a = 0; a < x; a++)
                {
                        _readFlog_2row(_conFlog, a, return_logTime, return_reason);
                        _conv_epoch(return_logTime, clock);
                        sprintf(msg, "log entry:[#%03d] Clk:[%s] Type:[%s]", a, clock, logTypes[return_reason]);
                        _post_msg(msg, 2);
                }
        }
}
void IPmonitoring::enter_fake_LOGentry(time_t t, uint8_t reason)
{
        char c[20];
        sprintf(c, "%d,%d", t, reason);
        _conFlog.write(c, true);
}
bool IPmonitoring::_startFlogs()
{
        return _conFlog.start(ENTRY_LENGTH, LOG_ENTRIES);
}
void IPmonitoring::_loopFlogs()
{
        _conFlog.looper();
}
void IPmonitoring::_LOGconnection()
{
        time_t t = time(nullptr);
        if (currentstateClk != 0)
        {
                _writeFlog_2row(_conFlog, 1, t, true); /* 1==reconnect */
        }
        else
        {
                _writeFlog_2row(_conFlog, 2, t, true); /* 2== boot & Pןing OK*/
        }
}
void IPmonitoring::_LOGdisconnection()
{
        time_t t = time(nullptr);
        _writeFlog_2row(_conFlog, 0, t, true);
}
void IPmonitoring::_readFlog_2row(flashLOG &LOG, int numLine, time_t &retTime, uint8_t &retType)
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
void IPmonitoring::_writeFlog_2row(flashLOG &LOG, uint8_t Reason, time_t value, bool writenow)
{
        /* Reason can be: 0: disconnect, 1: Reconnect, 2:Boot */
        char c[20];

        time_t t = time(nullptr);
        struct tm *tm = localtime(&t);

        if (tm->tm_year + 1900 == 1970)
        {
                _post_msg("NTP is not set - not entering logs entries", 1);
        }
        else
        {
                sprintf(c, "%d,%d", value, Reason);
                LOG.write(c, writenow);
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