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
// ~~~~~
