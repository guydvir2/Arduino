#include <myIOT2.h>
#include <Chrono.h>

#define NUM_CHRONOS 2

#include "myIOT_settings.h"

Chrono chron_0(Chrono::SECONDS);
Chrono chron_1(Chrono::SECONDS);
Chrono *chronVector[NUM_CHRONOS] = {&chron_0, &chron_1};

int timeouts[NUM_CHRONOS] = {0, 0};
int defaultTimeout[NUM_CHRONOS] = {30, 120};

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
long remainWatch(uint8_t i = 0)
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
void addWatch(uint8_t i, int _add)
{
  timeouts[i] += _add;
  notifyAdd(i, _add);

  if (!chronVector[i]->isRunning()) /* Case not ON */
  {
    ONcmd(i, timeouts[i]);
  }
}
void notifyAdd(uint8_t &i, int &_add)
{
  char a[50];
  sprintf(a, "Device [%d] Added [%d] seconds, total [%d] seconds", i, _add, timeouts[i]);
  iot.pub_msg(a);
  Serial.println(a);
}
void notifyOFF(uint8_t &i, int &_elapsed)
{
  char a[50];
  sprintf(a, "Device [%d] Switched [Off] after [%d] seconds", i, _elapsed);
  iot.pub_msg(a);
  Serial.println(a);
}
void notifyON(uint8_t &i)
{
  char a[50];
  sprintf(a, "Device [%d] Switched [On] for [%d] seconds", i, timeouts[i]);
  iot.pub_msg(a);
  Serial.println(a);
}
void ONcmd(uint8_t i, uint8_t _TO)
{
  if (_TO == 0)
  {
    timeouts[i] = defaultTimeout[i];
  }
  else
  {
    timeouts[i] = _TO;
  }
  startWatch(i);
  notifyON(i);
}
void OFFcmd(uint8_t i)
{
  int x = chronVector[i]->elapsed();
  notifyOFF(i, x);
  stopWatch(i);
}

void Chrono_looper()
{
  for (uint8_t i = 0; i < NUM_CHRONOS; i++)
  {
    if (chronVector[i]->isRunning() && chronVector[i]->hasPassed(timeouts[i]))
    {
      OFFcmd(i);
    }
  }
}
void stopAllWatches()
{
  for (uint8_t i = 0; i < NUM_CHRONOS; i++)
  {
    stopWatch(i);
  }
}

void setup()
{
  stopAllWatches();
  startIOTservices();
  ONcmd(0, 15);
}
void loop()
{
  iot.looper();
  delay(50);
  Chrono_looper();
}
