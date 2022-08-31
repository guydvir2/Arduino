#include <myWindowSW.h>
#include <myIOT2.h>

#define numW 1

WinSW *winSW_V[] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
#include "myIOT_settings.h"

void startWindow(uint8_t i)
{
  for (uint8_t x = 0; x < i; x++)
  {
    winSW_V[x] = new WinSW;
    winSW_V[x]->def(D5, D6, D3, D4);
    winSW_V[x]->def_extSW(D7, D2);
    winSW_V[x]->def_extras();
    winSW_V[x]->start();
  }
}
void loopWindow(uint8_t i)
{
  for (uint8_t x = 0; x < i; x++)
  {
    winSW_V[x]->loop();
    if (winSW_V[x]->newMSGflag)
    {
      _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
      winSW_V[x]->newMSGflag = false;
    }
  }
}

void setup()
{
  startIOTservices();
  startWindow(numW);
}

void loop()
{
  loopWindow(numW);
  iot.looper();
}
