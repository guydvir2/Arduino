#include <myIOT2.h>
#include <Button2.h>    /* Button Entities */
#include <RCSwitch.h>   /* Button Entities */
#include <myWindowSW.h> /* WinSW Entities */
#include "paramters.h"  /* Hardcoded or updated saved in flash */
#include "but_defs.h"
#include "myIOT_settings.h"

RCSwitch *RF_v = nullptr;

void init_WinSW()
{
  for (uint8_t x = 0; x < numW; x++)
  {
    winSW_V[x] = new WinSW;
    winSW_V[x]->def(WinputPins[x][0], WinputPins[x][1], WrelayPins[x][0], WrelayPins[x][1]);
    winSW_V[x]->def_extSW(WextInPins[x][0], WextInPins[x][1]);
    winSW_V[x]->def_extras(); /* Timeout, lockdown */
    winSW_V[x]->start();
    sprintf(winSW_V[x]->name, "%sWin%d", WIN_prefixTopic, x); /* Store MQTT topic inside LIB to avoid redefined overhead waste */
  }
}
void loop_WinSW()
{
  for (uint8_t x = 0; x < numW; x++)
  {
    winSW_V[x]->loop();
    // Button action MSG to main code
    if (winSW_V[x]->newMSGflag)
    {
      _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
      winSW_V[x]->newMSGflag = false;
      win_updateState(x, winSW_V[x]->MSG.state);
    }
    // END handler
  }
}

/* ******************* Buttons ******************* */
void init_buttons()
{
  init_butt(buttonPins, buttonTypes);
  init_outputs(relayPins);
  init_topics();
  init_RF();
}
void init_butt(uint8_t butPinArray[], bool butType[])
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    SW_v[i] = new SwitchStruct;
    SW_v[i]->id = i;

    if (SW_v[i]->useButton)
    {
      SW_v[i]->button.begin(butPinArray[i]);
      SW_v[i]->button.setID(i);
      SW_v[i]->virtCMD = false;

      if (butType[i] == 0) /* On-Off Switch */
      {
        SW_v[i]->button.setPressedHandler(OnOffSW_ON_handler);
        SW_v[i]->button.setReleasedHandler(OnOffSW_OFF_handler);
      }
      else /* Momentary press */
      {
        SW_v[i]->button.setPressedHandler(toggle_handle);
      }
    }
  }
}
void init_outputs(uint8_t relp[])
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    if (!SW_v[i]->virtCMD) /* virtCMD outputs a MQTT msg to operate other IOT device */
    {
      SW_v[i]->outPin = relp[i]; /* will be removed in future */
      pinMode(SW_v[i]->outPin, OUTPUT);
      digitalWrite(SW_v[i]->outPin, !OUTPUT_ON);
    }
  }
}
void init_topics()
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    sprintf(SW_v[i]->Topic, "%sLight%d", SW_prefixTopic, i);
  }
}
void init_RF()
{
  if (useRF)
  {
    RF_v = new RCSwitch();

    RF_v->enableReceive(RFpin);
  }
}

void loop_buttons()
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    if (SW_v[i]->useButton)
    {
      SW_v[i]->button.loop();
    }
  }
}
void loop_RF()
{
  if (useRF)
  {
    if (RF_v->available())
    {
      // sprintf(temp, "Received %d / %dbit Protocol: ", RFreader.getReceivedValue(), RFreader.getReceivedBitlength(), RFreader.getReceivedProtocol());
      static unsigned long lastEntry = 0;
      for (uint8_t i = 0; i < sizeof(RF_keyboardCode) / sizeof(RF_keyboardCode[0]); i++)
      {
        if (RF_keyboardCode[i] == RF_v->getReceivedValue() && millis() - lastEntry > 1000)
        {
          toggleRelay(i, RF);
          lastEntry = millis();
        }
      }
      RF_v->resetAvailable();
    }
  }
}
/* ************************************************ */

void boot_summary()
{
  Serial.print("numWindows:\t\t");
  Serial.println(numW);
  Serial.print("Topics:\t\t");
  for (uint8_t i = 0; i < numW; i++)
  {
    // Serial.print(winTopics[i]);
    Serial.print("; ");
  }
  Serial.println();

  Serial.print("numSwitches:\t\t");
  Serial.println(numSW);
  Serial.print("Topics:\t\t");
  for (uint8_t i = 0; i < numSW; i++)
  {
    // Serial.print(winTopics[i]);
    // Serial.print("; ");
  }
  Serial.println();
}
void setup()
{
  updateTopics();
  init_WinSW();
  init_buttons();
  startIOTservices();
}
void loop()
{
  loop_RF();
  loop_WinSW();
  loop_buttons();
  iot.looper();
}
