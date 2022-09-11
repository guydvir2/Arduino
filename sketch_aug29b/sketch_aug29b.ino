#include <myIOT2.h>
#include <Button2.h>    /* Button Entities */
#include <RCSwitch.h>   /* Button Entities */
#include <myWindowSW.h> /* WinSW Entities */
#include "paramters.h"  /* Hardcoded or updated saved in flash */
#include "but_defs.h"
#include "myIOT_settings.h"

RCSwitch *RF_v = nullptr;

/* ******************* Windows ******************* */
void print_win_instance(WinSW &winsw)
{
  Serial.println("<<<<<<<<<<<< Window Entity >>>>>>>>>>>>>>");
  Serial.print("ID:\t");
  Serial.println(winsw.id);
  Serial.print("upPin:\t");
  Serial.println(winsw._outpins[0]);
  Serial.print("downPin:\t");
  Serial.println(winsw._outpins[1]);
  Serial.print("extSW:\t");
  Serial.println(winsw._useExtSW);
  Serial.print("virtCMD:\t");
  Serial.println(winsw._virtWin);
  Serial.print("name:\t");
  Serial.println(winsw.name);
  yield();
}
void create_WinSW_instance(JsonDocument &_DOC, uint8_t i)
{
  winSW_V[winEntityCounter] = new WinSW;

  if (strcmp(_DOC["virtCMD"][i], "") != 0)
  {
    strlcpy(winSW_V[winEntityCounter]->name, _DOC["virtCMD"][i], TOPIC_LEN);
    winSW_V[winEntityCounter]->def(_DOC["inputPins"][lastUsed_inIO], _DOC["inputPins"][lastUsed_inIO + 1]);
  }
  else
  {
    winSW_V[winEntityCounter]->def(_DOC["inputPins"][lastUsed_inIO], _DOC["inputPins"][lastUsed_inIO + 1], _DOC["relayPins"][lastUsed_outIO], _DOC["relayPins"][lastUsed_outIO + 1]);
    lastUsed_outIO += 2;
  }

  if (_DOC["extInputs"][i] == 1) /* define a Secondary input for a window */
  {
    winSW_V[winEntityCounter]->def_extSW(_DOC["inputPins"][lastUsed_inIO + 2], _DOC["inputPins"][lastUsed_inIO + 3]);
    lastUsed_inIO += 2;
  }
  winSW_V[winEntityCounter]->def_extras(); /* Timeout, lockdown */
  winSW_V[winEntityCounter]->start();

  print_win_instance(*winSW_V[winEntityCounter]);

  winEntityCounter++; /* inc Windows Enetity counter */
  lastUsed_inIO += 2;
}

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
    // §§§§§§ Button action MSG to main code §§§§§§§
    if (winSW_V[x]->newMSGflag)
    {
      _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
      winSW_V[x]->newMSGflag = false;
    }
    // §§§§§§§§§§§§§ END handler §§§§§§§§§§§§§§§§§
  }
}
/* ************************************************ */

/* ******************* Buttons ******************* */
void create_SW_instance(JsonDocument &_DOC, uint8_t i)
{
  uint8_t a = _DOC["ButtonTypes"][i];
  SW_v[swEntityCounter] = new SwitchStruct;
  SW_v[swEntityCounter]->id = swEntityCounter;

  if (a > 0) /* Any input */
  {
    SW_v[swEntityCounter]->button.begin(_DOC["inputPins"][lastUsed_inIO]);
    SW_v[swEntityCounter]->button.setID(swEntityCounter);

    if (a == 1) /* On-Off Switch */
    {
      SW_v[swEntityCounter]->button.setPressedHandler(OnOffSW_ON_handler);
      SW_v[swEntityCounter]->button.setReleasedHandler(OnOffSW_OFF_handler);
    }
    else if (a == 2) /* Momentary/ Push button press */
    {
      SW_v[swEntityCounter]->button.setPressedHandler(toggle_handle);
    }
  }

  if (strcmp(_DOC["virtCMD"][i], "") != 0) /* Virtual CMD */
  {
    SW_v[swEntityCounter]->virtCMD = true;
    strlcpy(SW_v[swEntityCounter]->Topic, _DOC["virtCMD"][i].as<const char *>(), TOPIC_LEN);
  }
  else /* Assign output to relays - Physical SW */
  {
    SW_v[swEntityCounter]->virtCMD = false;
    SW_v[swEntityCounter]->outPin = _DOC["relayPins"][lastUsed_outIO];

    pinMode(SW_v[swEntityCounter]->outPin, OUTPUT);
    digitalWrite(SW_v[swEntityCounter]->outPin, !OUTPUT_ON);

    lastUsed_outIO++;
  }

  print_sw_struct(*SW_v[swEntityCounter]);

  lastUsed_inIO++;
  swEntityCounter++;
}
void init_buttons()
{
  for (uint8_t i = 0; i < numSW; i++)
  {
    init_butt(buttonPins, buttonTypes, i);
    init_outputs(relayPins, i);
    init_RF(i);
  }
}
void init_butt(uint8_t butPinArray[], bool butType[], uint8_t i)
{
  SW_v[i] = new SwitchStruct; /* add new instance of Switch entity */
  SW_v[i]->id = i;            /* ID number*/

  if (SW_v[i]->useButton) /* Entity can have an input that triggers (or MQTT command only) */
  {
    SW_v[i]->button.begin(butPinArray[i]);
    SW_v[i]->button.setID(i);
    SW_v[i]->virtCMD = false; /* IO command calls MQTT rather than trigger a relay */

    if (butType[i] == 0) /* On-Off Switch */
    {
      SW_v[i]->button.setPressedHandler(OnOffSW_ON_handler);
      SW_v[i]->button.setReleasedHandler(OnOffSW_OFF_handler);
    }
    else /* Momentary/ Push button press */
    {
      SW_v[i]->button.setPressedHandler(toggle_handle);
    }
  }
}
void init_outputs(uint8_t relp[], uint8_t i)
{
  if (!SW_v[i]->virtCMD) /* virtCMD outputs a MQTT msg to operate other IOT device */
  {
    SW_v[i]->outPin = relp[i]; /* will be removed in future */
    pinMode(SW_v[i]->outPin, OUTPUT);
    digitalWrite(SW_v[i]->outPin, !OUTPUT_ON);
  }
}
void init_RF(uint8_t i)
{
  if (SW_v[i]->RFch != 255 && RF_v == nullptr)
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
  if (RF_v->available()) /* New transmission */
  {
    // sprintf(temp, "Received %d / %dbit Protocol: ", RFreader.getReceivedValue(), RFreader.getReceivedBitlength(), RFreader.getReceivedProtocol());
    static unsigned long lastEntry = 0;
    for (uint8_t i = 0; i < sizeof(RF_keyboardCode) / sizeof(RF_keyboardCode[0]); i++)
    {
      if (RF_keyboardCode[i] == RF_v->getReceivedValue() && millis() - lastEntry > 1000)
      {
        for (uint8_t x = 0; x < numSW; x++) /* choose the right switch to the received code */
        {
          toggleRelay(x, RF);
          lastEntry = millis();
        }
      }
    }
    RF_v->resetAvailable();
  }
}
/* ************************************************ */

void setup()
{
  startIOTservices();
}
void loop()
{
  // loop_RF();
  // loop_WinSW();
  // loop_buttons();
  iot.looper();
}
