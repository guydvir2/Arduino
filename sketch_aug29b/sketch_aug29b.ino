#include <myIOT2.h>
#include <Button2.h>    /* Button Entities */
#include <RCSwitch.h>   /* Button Entities */
#include <myWindowSW.h> /* WinSW Entities */
#include "paramters.h"  /* Hardcoded or updated saved in flash */
#include "but_defs.h"
#include "myIOT_settings.h"

RCSwitch RFreader = RCSwitch();

void init_WinSW()
{
  for (uint8_t x = 0; x < numW; x++)
  {
    winSW_V[x] = new WinSW;
    winSW_V[x]->def(WinputPins[x][0], WinputPins[x][1], WrelayPins[x][0], WrelayPins[x][1]);
    winSW_V[x]->def_extSW(WextInPins[x][0], WextInPins[x][1]);
    winSW_V[x]->def_extras(); /* Timeout, lockdown */
    winSW_V[x]->start();
  }
}
void loop_WinSW()
{
  for (uint8_t x = 0; x < numW; x++)
  {
    winSW_V[x]->loop();
    if (winSW_V[x]->newMSGflag)
    {
      _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
      winSW_V[x]->newMSGflag = false;
      win_updateState(x, winSW_V[x]->MSG.state);
    }
  }
}

/* ******************* Buttons ******************* */
void init_buttons()
{
  init_butt();
  init_outputs();
  init_RF();
}
void init_butt()
{
  if (useButton)
  {
    for (uint8_t i = 0; i < numSW; i++)
    {
      Buttons[i] = new Button2;
      Buttons[i]->begin(buttonPins[i]);
      if (buttonTypes[i] == 0) /* On-Off Switch */
      {
        Buttons[i]->setPressedHandler(OnOffSW_ON_handler);
        Buttons[i]->setReleasedHandler(OnOffSW_OFF_handler);
      }
      else if (buttonTypes[i] == 1) /* Momentary press */
      {
        Buttons[i]->setPressedHandler(toggle_handle);
      }
      Buttons[i]->setID(i);
    }
  }
}
void init_outputs()
{
  for (byte i = 0; i < numSW; i++)
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], !OUTPUT_ON);
  }
}
void init_RF()
{
  if (useRF)
  {
    RFreader.enableReceive(RFpin);
  }
}

void loop_buttons()
{
  if (useButton)
  {
    for (byte i = 0; i < numSW; i++)
    {
      Buttons[i]->loop();
    }
  }
}
void loop_RF()
{
  if (useRF)
  {
    if (RFreader.available())
    {
      // sprintf(temp, "Received %d / %dbit Protocol: ", RFreader.getReceivedValue(), RFreader.getReceivedBitlength(), RFreader.getReceivedProtocol());
      static unsigned long lastEntry = 0;
      for (uint8_t i = 0; i < sizeof(RF_keyboardCode) / sizeof(RF_keyboardCode[0]); i++)
      {
        if (RF_keyboardCode[i] == RFreader.getReceivedValue() && millis() - lastEntry > 1000)
        {
          toggleRelay(i, RF);
          lastEntry = millis();
        }
      }
      RFreader.resetAvailable();
    }
  }
}
/* ************************************************ */

void boot_summary()
{
  Serial.print("numWindows:\t\t");
  Serial.println(numW);
  Serial.print("Topics:\t\t") for (uint8_t i = 0; i < NUMW; i++)
  {
    Serial.print(winTopics[i]);
    Serial.print(\t);
  }

  Serial.print("numSwitches:\t\t");
  Serial.println(numSW);
}
void setup()
{
  updateTopics();
  startIOTservices();
  init_WinSW();
  init_buttons();
}
void loop()
{
  loop_RF();
  loop_WinSW();
  loop_buttons();
  iot.looper();
}
