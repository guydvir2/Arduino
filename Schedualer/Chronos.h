void startWatch(uint8_t i = 0)
{
    chronVector[i]->restart();
}
void stopWatch(uint8_t i = 0)
{
    chronVector[i]->restart();
    chronVector[i]->stop();
    timeouts[i] = 0;
}
unsigned int remainWatch(uint8_t i = 0)
{
    if (chronVector[i]->isRunning())
    {
        return chronVector[i]->elapsed() / 1000;
    }
    else
    {
        return 0;
    }
}
void addWatch(uint8_t i, int _add, const char *trigger = nullptr)
{
    timeouts[i] += _add;
    notifyAdd(i, _add, trigger);

    if (!chronVector[i]->isRunning()) /* Case not ON */
    {
        ONcmd(i, timeouts[i], trigger);
    }
}
void stopAllWatches()
{
    for (uint8_t i = 0; i < numSW; i++)
    {
        stopWatch(i);
    }
}
void loopAllWatches()
{
    for (uint8_t i = 0; i < numSW; i++)
    {
        if (chronVector[i]->isRunning() && chronVector[i]->hasPassed(timeouts[i]))
        {
            OFFcmd(i, SRCS[TIMEOUT]);
        }
    }
}