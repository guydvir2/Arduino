#include <myIOT2.h>
#include <Button2.h>    /* Button Entities */
#include <RCSwitch.h>   /* Button Entities */
#include <myWindowSW.h> /* WinSW Entities */
#include "paramters.h"  /* Hardcoded or updated saved in flash */
#include "but_defs.h"
#include "myIOT_settings.h"


/* ******************* Windows ******************* */
void print_win_instance(WinSW &winsw)
{
  Serial.println("<<<<<<<<<<<< Window Entity >>>>>>>>>>>>>>");
  Serial.print("ID:\t");
  Serial.println(winsw.id);
  Serial.print("upPin:\t");
  Serial.println(winsw.outpins[0]);
  Serial.print("downPin:\t");
  Serial.println(winsw.outpins[1]);
  Serial.print("extSW:\t");
  Serial.println(winsw._useExtSW);
  Serial.print("virtCMD:\t");
  Serial.println(winsw.virtCMD);
  Serial.print("name:\t");
  Serial.println(winsw.name);
  yield();
}
void create_WinSW_instance(JsonDocument &_DOC, uint8_t i)
{
  winSW_V[winEntityCounter] = new WinSW;

  if (strcmp(_DOC["virtCMD"][i], "") == 0) /* a virtCMD on output */
  {
    strlcpy(winSW_V[winEntityCounter]->name, _DOC["virtCMD"][i], TOPIC_LEN);
    winSW_V[winEntityCounter]->virtCMD = true;
    winSW_V[winEntityCounter]->def(_DOC["inputPins"][lastUsed_inIO], _DOC["inputPins"][lastUsed_inIO + 1]);
  }
  else /* Physical Switching input & output */
  {
    winSW_V[winEntityCounter]->def(_DOC["inputPins"][lastUsed_inIO], _DOC["inputPins"][lastUsed_inIO + 1], _DOC["relayPins"][lastUsed_outIO], _DOC["relayPins"][lastUsed_outIO + 1]);
    lastUsed_outIO += 2;
  }

  if (_DOC["WextInputs"][i] == 1) /* define a Secondary input for a window */
  {
    winSW_V[winEntityCounter]->def_extSW(_DOC["inputPins"][lastUsed_inIO + 2], _DOC["inputPins"][lastUsed_inIO + 3]);
    lastUsed_inIO += 2;
  }

  winSW_V[winEntityCounter]->def_extras(); /* Timeout & lockdown */
  winSW_V[winEntityCounter]->start();

  print_win_instance(*winSW_V[winEntityCounter]); /* debug only */

  winEntityCounter++; /* inc Windows Enetity counter */
  lastUsed_inIO += 2;
}
void _Win_virtCMD(uint8_t state, uint8_t reason, uint8_t x)
{
  if (winSW_V[x]->virtCMD == true)
  {
    iot.pub_noTopic((char *)winMQTTcmds[state], winSW_V[x]->name); // <---- Fix this
  }
}
void _newMSGcb(uint8_t x)
{
  if (winSW_V[x]->newMSGflag)
  {
    _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x); /* Generate MQTT MSG */
    _Win_virtCMD(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
    winSW_V[x]->newMSGflag = false;
  }
}
void loop_WinSW()
{
  for (uint8_t x = 0; x < winEntityCounter; x++)
  {
    winSW_V[x]->loop();
    _newMSGcb(x);
  }
}
/* ************************************************ */

/* ******************* Buttons ******************* */
void create_SW_instance(JsonDocument &_DOC, uint8_t i)
{
  uint8_t a = _DOC["SW_buttonTypes"][i];
  SW_v[swEntityCounter] = new SwitchStruct;
  SW_v[swEntityCounter]->id = swEntityCounter;

  if (a > 0) /* Any input */
  {
    SW_v[swEntityCounter]->useButton = true;
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

  if (strcmp(_DOC["virtCMD"][i], "") == 0) /* Virtual CMD */
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
void loop_buttons()
{
  for (uint8_t i = 0; i < swEntityCounter; i++)
  {
    if (SW_v[i]->useButton)
    {
      SW_v[i]->button.loop();
    }
  }
}
/* ************************************************ */

/* ******************* RF 433Hz ******************* */
RCSwitch *RF_v = nullptr;

void init_RF(uint8_t i)
{
  if (SW_v[i]->RFch != 255 && RF_v == nullptr)
  {
    RF_v = new RCSwitch();
    RF_v->enableReceive(RFpin);
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
        for (uint8_t x = 0; x < swEntityCounter; x++) /* choose the right switch to the received code */
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
  if (paramLoadedOK)
  {
    loop_RF();
    loop_WinSW();
    loop_buttons();
  }
  iot.looper();
}
