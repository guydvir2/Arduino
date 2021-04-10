#define ENTRY_LENGTH 200
#define LOG_ENTRIES 50

flashLOG connectionLOG("/conlog.txt");
flashLOG disconnectionLOG("/disconlog.txt");

unsigned long connLOG[LOG_ENTRIES];
unsigned long disconnLOG[LOG_ENTRIES];


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
// void deleteFlog(flashLOG &LOG)
// {
//     LOG.delog();
// }
void flog_init()
{
    connectionLOG.start(ENTRY_LENGTH, LOG_ENTRIES);
    disconnectionLOG.start(ENTRY_LENGTH, LOG_ENTRIES);
}
void check_log_onBoot()
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
void startFlogs()
{
    flog_init();
    // check_log_onBoot()
}
void loopFlogs()
{
    disconnectionLOG.looper();
    connectionLOG.looper();
}

// ±±±±±±±±±± LOG functions ±±±±±±±±±±±±±±
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
bool verifyLOG()
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
    if (internet_monitor.isConnected)
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
    int c = getLOG_ENTRIES(connLOG);
    int d = getLOG_ENTRIES(disconnLOG);

    if (internet_monitor.isConnected)
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
int driftLOG(unsigned long LOG[])
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
void updateLOG(unsigned long LOG[], unsigned long now = now())
{

    int i= driftLOG(LOG); /* If log is full- swap is done */
    // int i = getLOG_ENTRIES(LOG);
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
// ~~~~~
