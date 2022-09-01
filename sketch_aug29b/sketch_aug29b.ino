#include <myIOT2.h>

#define USE_WINDOWS true

#if USE_WINDOWS
#include <myWindowSW.h>
#define maxW 4

WinSW *winSW_V[maxW] = {nullptr, nullptr, nullptr, nullptr};

uint8_t numW = 2;
uint8_t WrelayPins[maxW][2] = {{D1, D2}, {D5, D6}, {0, 0}, {0, 0}};
uint8_t WinputPins[maxW][2] = {{D3, D4}, {D7, D8}, {0, 0}, {0, 0}};
uint8_t WextInPins[maxW][2] = {{255, 255}, {255, 255}, {255, 255}, {255, 255}};

#include "myIOT_settings.h"

void startWindow()
{
  for (uint8_t x = 0; x < numW; x++)
  {
    winSW_V[x] = new WinSW;
    winSW_V[x]->def(WinputPins[x][0], WinputPins[x][1], WinputPins[x][0], WinputPins[x][1]);
    winSW_V[x]->def_extSW(WextInPins[x][0], WextInPins[x][1]);
    winSW_V[x]->def_extras();
    winSW_V[x]->start();
  }
}
void loopWindow()
{
  for (uint8_t x = 0; x < numW; x++)
  {
    winSW_V[x]->loop();
    if (winSW_V[x]->newMSGflag)
    {
      _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
      winSW_V[x]->newMSGflag = false;
    }
  }
}
#endif

void setup()
{
  startIOTservices();
#if USE_WINDOWS
  startWindow();
#endif
}

void loop()
{
#if USE_WINDOWS
  loopWindow();
#endif
  iot.looper();
}
