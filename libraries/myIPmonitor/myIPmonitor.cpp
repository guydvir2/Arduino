#include <myIPmonitor.h>
#include <Arduino.h>
#include <myLOG.h>

IPmonitoring::IPmonitoring(char *IP, char *nick)
    : _IP(IP), _nick(nick), _conlog_filename(_str_concat(nick, "_connlog.txt")), _conFlog(_conlog_filename)
{
}
IPmonitoring::~IPmonitoring()
{
        delete[] _conlog_filename;
}
void IPmonitoring::start(cb_func ping)
{
        char a[80];
        _ping_cb = ping;
        if (_verify_internet_ok())
        {
                bootClk = now();
                sprintf(a, "pings %s", _IP);
                _post_msg(a);
                if (!_startFlogs())
                {
                        _post_msg("Start LOG failed!");
                }
        }
        else
        {
                sprintf(a, "%s NOT Connected. Internet/NTP Failure", _nick);
                _post_msg(a);
        }
}
void IPmonitoring::loop()
{
        _reset_bootFailure();
        _ping_looper();
        _loopFlogs();
}
void IPmonitoring::printFlog(int i)
{
        if (i == NULL)
        {
                byte retR;
                time_t retT;
                char msg[50];
                char clock[25];
                const char *logTypes[] = {"Disconnect", "Reconnect", "Boot"};
                int x = _conFlog.getnumlines();

                for (int a = 0; a < x; a++)
                {
                        _readFlog_2row(_conFlog, a, retT, retR);
                        _conv_epoch(retT, clock);
                        sprintf(msg, "log entry:[#%03d] Clk:[%s] Type:[%s]", a, clock, logTypes[retR]);
                        _post_msg(msg);
                }
        }
}
void IPmonitoring::getStatus(int h)
{
        byte retR1;
        byte retR2;
        time_t retT1;
        time_t retT2;
        const unsigned long hrs2sec = 60 * 60 * h;

        char connect_duration[20];
        char disco_duration[20];
        char a[100];
        unsigned int cum_dTime = 0;
        unsigned int cum_cTime = 0;
        unsigned int total_c = 0;
        unsigned int total_disc = 0;
        unsigned int total_boots = 0;

        const unsigned int total_records = _conFlog.getnumlines();
        for (int i = 0; i < total_records - 1; i++)
        {
                /* retT2 is log entry after retT1 */
                _readFlog_2row(_conFlog, i, retT1, retR1);
                if (now() - retT1 <= hrs2sec)
                {
                        _readFlog_2row(_conFlog, i + 1, retT2, retR2);

                        if (retR1 == 0 && retR2 != 0)
                        {
                                cum_dTime += retT2 - retT1;
                                total_disc++;
                        }
                        else if (retR1 == 1 && retR2 != 1)
                        {
                                cum_cTime += retT2 - retT1;
                                total_c++;
                        }
                        else if (retR1 == 2)
                        {
                                cum_cTime += retT2 - retT1;
                                total_boots++;
                        }
                        else
                        {
                                Serial.print("Err on record #");
                                Serial.println(i);
                        }
                }
        }
        _readFlog_2row(_conFlog, total_records - 1, retT1, retR1);
        if (retR1 == 2)
        {
                cum_cTime += now() - retT1;
                total_boots++;
        }
        else if (retR1 == 1)
        {
                cum_cTime += now() - retT1;
                total_c++;
        }
        else
        {
                cum_dTime += now() - retT1;
                total_disc++;
        }
        _conv_epoch_duration(cum_cTime, 0, connect_duration);
        _conv_epoch_duration(cum_dTime, 0, disco_duration);
        sprintf(a, "Records: Total/ Disconnects/ Reconnects/ Reboots:[%d/ %d/ %d/ %d]", total_records, total_disc, total_c, total_boots);
        _post_msg(a);
        sprintf(a, "Durations: Disconnected/Connected:[%s/ %s]", disco_duration, connect_duration);
        _post_msg(a);
}
void IPmonitoring::deleteLOG()
{
        _conFlog.delog();
}
void IPmonitoring::_disco_service()
{
        char a[50];
        currentstateClk = now();
        dCounter++;
        _LOGdisconnection();
        sprintf(a, "%s disconnect [#%d]", _nick, dCounter);
        _post_msg(a);
}
void IPmonitoring::_reco_service()
{
        if (currentstateClk != 0)
        {
                char a[50];
                char b[20];
                char c[10];
                sprintf(a, "%s reconnect [#%d] after", _nick, dCounter);
                _post_msg(a);
        }
        else
        {
                _post_msg(_nick, " connected");
        }
        _LOGconnection();
        currentstateClk = now();
}
bool IPmonitoring::_ping_client()
{
        byte retries = 3;
        byte retCounter = 0;
        bool pingOK = false;

        while (pingOK == false && retCounter < retries)
        {
                if (WiFi.status() == WL_CONNECTED)
                {
                        pingOK = _ping_cb(_IP, 3);
                }
                retCounter++;
        }

        if (pingOK != isConnected || _firstPing == true)
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
                _firstPing = false;
        }
        return isConnected;
}
void IPmonitoring::_ping_looper()
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
                        if (_pingCounter != 0)
                        {
                                _pingCounter = 0;
                                _adaptive_ping_val = MINPING_TIME;
                                _post_msg("Min_PING");
                        }
                }
                if (_pingCounter == pingsOK)
                {
                        _adaptive_ping_val = MAXPING_TIME;
                        _post_msg("Max_PING");
                }
        }
}
bool IPmonitoring::_verify_internet_ok()
{
        time_t t = now();
        if (WiFi.status() == WL_CONNECTED && year(t) != 1970)
        {
                _needRESET = false;
                return 1;
        }
        else
        {
                return 0;
        }
}
void IPmonitoring::_conv_epoch(time_t t, char *retDate)
{
        sprintf(retDate, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
}
void IPmonitoring::_conv_epoch_duration(long t1, long t2, char *clk)
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

        if (days != 0)
        {
                sprintf(clk, "%dd %02d:%02d:%02d", days, hours, minutes, seconds);
        }
        else
        {
                sprintf(clk, "%02d:%02d:%02d", hours, minutes, seconds);
        }
}
void IPmonitoring::_post_msg(char *inmsg, char *inmsg2)
{
        String msg;
        char Clk[20];
        _conv_epoch(now(), Clk);
        msg = "[" + String(Clk) + "]" + "[" + String(_nick) + "] " + String(inmsg) + String(inmsg2);
        Serial.println(msg);
}
void IPmonitoring::_reset_bootFailure()
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
                                _post_msg("No internet on Boot");
                                ESP.reset();
                                delay(1000);
                        }
                }
        }
}

// ~~~~~~~~~~FlashLOGS
void IPmonitoring::_LOGconnection()
{
        if (currentstateClk != 0)
        {
                _writeFlog_2row(_conFlog, 1, now(), true); /* 1==reconnect */
        }
        else
        {
                _writeFlog_2row(_conFlog, 2, now(), true); /* 2==boot */
        }
}
void IPmonitoring::_LOGdisconnection()
{
        _writeFlog_2row(_conFlog, 0, now(), true);
}
void IPmonitoring::_readFlog_2row(flashLOG &LOG, int numLine, time_t &retTime, byte &retType)
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
void IPmonitoring::_writeFlog_2row(flashLOG &LOG, byte Reason, time_t value, bool writenow)
{
        char c[20];
        time_t t = now();

        if (year(t) == 1970)
        {
                _post_msg("NTP is not set - not entering logs entries");
        }
        else
        {
                sprintf(c, "%d,%d", value, Reason);
                LOG.write(c);
                if (writenow)
                {
                        LOG.writeNow();
                }
        }
        // printFlog();
}
bool IPmonitoring::_startFlogs()
{
        return _conFlog.start(ENTRY_LENGTH, LOG_ENTRIES);
}
void IPmonitoring::_loopFlogs()
{
        _conFlog.looper();
}

// ~~~~~~~~~ Utils ~~~~~~~~~~
char *IPmonitoring::_str_concat(const char *a, const char *b)
{
        char *s = new char[strlen(a) + strlen(b) + 1];
        strcpy(s, a);
        strcat(s, b);
        return s;
}
int IPmonitoring::_inline_read(char *inputstr)
{
        char *pch;
        byte i = 0;
        const byte MAX_P = 3;

        pch = strtok(inputstr, " ,.-");
        while (pch != NULL && i < MAX_P)
        {
                sprintf(_inline_param[i], "%s", pch);
                pch = strtok(NULL, " ,.-");
                i++;
        }
        return i;
}