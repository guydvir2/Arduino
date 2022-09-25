#include <myIOT2.h>
#include <Button2.h>    /* Button Entities */
#include <RCSwitch.h>   /* Button Entities */
#include <myWindowSW.h> /* WinSW Entities */
#include "paramters.h"  /* Hardcoded or updated saved in flash */
#include "myIOT_settings.h"

/* ******************* Windows ******************* */
void create_WinSW_instance(JsonDocument &_DOC, uint8_t i)
{
  winSW_V[winEntityCounter] = new WinSW;

  Serial.print(" >>>>>>>>>>>>>>> Window #:\t");
  Serial.print(winEntityCounter);
  Serial.println(" <<<<<<<<<<<<<<<<<< ");

  // <<<<<<<<<<< Define input and output pins >>>>>>>>>>>>>>
  if (strcmp(_DOC["virtCMD"][i], "") != 0) /* a virtCMD on output */
  {
    winSW_V[winEntityCounter]->virtCMD = true;
    winSW_V[winEntityCounter]->def(inPinsArray[lastUsed_inIO], inPinsArray[lastUsed_inIO + 1]);

    Serial.print("virtCMD :\t");
    Serial.println(winSW_V[winEntityCounter]->virtCMD);
    Serial.print("in_pins #:\t");
    Serial.print(inPinsArray[lastUsed_inIO]);
    Serial.print("; ");
    Serial.println(inPinsArray[lastUsed_inIO + 1]);
    Serial.println("out_pins #:\t NONE");
  }
  else /* Physical Switching input & output */
  {
    winSW_V[winEntityCounter]->def(inPinsArray[lastUsed_inIO], inPinsArray[lastUsed_inIO + 1], outPinsArray[lastUsed_outIO], outPinsArray[lastUsed_outIO + 1]);
    winSW_V[winEntityCounter]->virtCMD = false;

    Serial.print("virtCMD :\t");
    Serial.println(winSW_V[winEntityCounter]->virtCMD);
    Serial.print("in_pins #:\t");
    Serial.print(inPinsArray[lastUsed_inIO]);
    Serial.print("; ");
    Serial.println(inPinsArray[lastUsed_inIO + 1]);
    Serial.print("out_pins #:\t");
    Serial.print(outPinsArray[lastUsed_outIO]);
    Serial.print("; ");
    Serial.println(outPinsArray[lastUsed_outIO + 1]);

    lastUsed_outIO += 2;
  }

  // <<<<<<<<<<< Define Ext_input pins , if needed >>>>>>>>>>>>>>
  if (_DOC["WextInputs"][i] == 1) /* define a Secondary input for a window */
  {
    winSW_V[winEntityCounter]->def_extSW(inPinsArray[lastUsed_inIO + 2], inPinsArray[lastUsed_inIO + 3]);

    Serial.print("ext_pins #:\t");
    Serial.print(inPinsArray[lastUsed_inIO + 2]);
    Serial.print("; ");
    Serial.println(inPinsArray[lastUsed_inIO + 3]);

    lastUsed_inIO += 2;
  }

  // <<<<<<<<<<< Init instance  >>>>>>>>>>>>>>
  winSW_V[winEntityCounter]->def_extras(); /* Timeout & lockdown */
  winSW_V[winEntityCounter]->start();

  // <<<<<<<<< Incrementing Counters >>>>>>>>>>
  winEntityCounter++;
  lastUsed_inIO += 2;
}
void _Win_virtCMD(uint8_t state, uint8_t reason, uint8_t x)
{
  if (winSW_V[x]->virtCMD == true)
  {
    iot.pub_noTopic((char *)winMQTTcmds[state], winSW_V[x]->name); // <---- Fix this : off cmd doesnot appear
  }
}
void _newMSGcb(uint8_t x)
{
  if (winSW_V[x]->newMSGflag)
  {
    _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);  /* Generate MQTT MSG */
    _Win_virtCMD(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x); /* Sending MQTT cmd in case of virt Win*/
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
bool _isON(uint8_t i)
{
  return digitalRead(SW_v[i]->outPin) == OUTPUT_ON;
}
void SW_turnON_cb(uint8_t i, uint8_t type)
{
  if (!SW_v[i]->virtCMD)
  {
    if (!_isON(i))
    {
      HWturnON(SW_v[i]->outPin);
      _gen_ButtMSG(i, type, HIGH);
    }
    else
    {
      Serial.print(i);
      Serial.println(" Already on");
    }
  }
  else
  {
    iot.pub_noTopic(buttMQTTcmds[0], SW_v[i]->Topic);
    _gen_ButtMSG(i, type, HIGH);
  }
}
void SW_turnOFF_cb(uint8_t i, uint8_t type)
{
  if (!SW_v[i]->virtCMD)
  {
    if (_isON(i))
    {
      HWturnOFF(SW_v[i]->outPin);
      _gen_ButtMSG(i, type, LOW);
    }
    else
    {
      Serial.print(i);
      Serial.println(" Already off");
    }
  }
  else
  {
    iot.pub_noTopic(buttMQTTcmds[1], SW_v[i]->Topic);
    _gen_ButtMSG(i, type, LOW);
  }
}
void OnOffSW_Relay(uint8_t i, bool state, uint8_t type)
{
  if (state == true)
  {
    SW_turnON_cb(i, type);
  }
  else
  {
    SW_turnOFF_cb(i, type);
  }
}
void toggleRelay(uint8_t i, uint8_t type)
{
  if (!SW_v[i]->virtCMD)
  {
    if (_isON(i))
    {
      SW_turnOFF_cb(i, type);
    }
    else
    {
      SW_turnON_cb(i, type);
    }
  }
  else
  {
    if (SW_v[i]->guessState == true)
    {
      SW_turnOFF_cb(i, type);
    }
    else
    {
      SW_turnON_cb(i, type);
    }
    SW_v[i]->guessState = !SW_v[i]->guessState;
  }
}

/* Button2 Handlers */
void OnOffSW_ON_handler(Button2 &b)
{
  OnOffSW_Relay(b.getID(), OUTPUT_ON, _MQTT);
}
void OnOffSW_OFF_handler(Button2 &b)
{
  OnOffSW_Relay(b.getID(), !OUTPUT_ON, _MQTT);
}
void toggle_handle(Button2 &b)
{
  toggleRelay(b.getID(), _MQTT);
}

void create_SW_instance(JsonDocument &_DOC, uint8_t i)
{
  SW_v[swEntityCounter] = new SwitchStruct;
  SW_v[swEntityCounter]->id = swEntityCounter;
  SW_v[swEntityCounter]->type = _DOC["SW_buttonTypes"][i] | 1;

  if (SW_v[swEntityCounter]->type > 0) /* Has any input */
  {
    SW_v[swEntityCounter]->useButton = true;
    SW_v[swEntityCounter]->button.begin(inPinsArray[lastUsed_inIO]);
    SW_v[swEntityCounter]->button.setID(swEntityCounter);

    if (SW_v[swEntityCounter]->type == 1) /* On-Off Switch */
    {
      SW_v[swEntityCounter]->button.setPressedHandler(OnOffSW_ON_handler);
      SW_v[swEntityCounter]->button.setReleasedHandler(OnOffSW_OFF_handler);
    }
    else if (SW_v[swEntityCounter]->type == 2) /* Momentary/ Push button press */
    {
      SW_v[swEntityCounter]->button.setPressedHandler(toggle_handle);
    }

    lastUsed_inIO++;
  }
  else /* Entity without physical input */
  {
    SW_v[swEntityCounter]->useButton = false;
  }

  if (strcmp(_DOC["virtCMD"][i], "") != 0) /* Virtual CMD */
  {
    SW_v[swEntityCounter]->virtCMD = true;
    strlcpy(SW_v[swEntityCounter]->Topic, _DOC["virtCMD"][i].as<const char *>(), MAX_TOPIC_SIZE);
  }
  else /* Assign output to relays - Physical SW */
  {
    SW_v[swEntityCounter]->virtCMD = false;
    SW_v[swEntityCounter]->outPin = outPinsArray[lastUsed_outIO];

    pinMode(SW_v[swEntityCounter]->outPin, OUTPUT);
    digitalWrite(SW_v[swEntityCounter]->outPin, !OUTPUT_ON);

    lastUsed_outIO++;
  }

  if (_DOC["RF_2entity"][swEntityCounter] != 255)
  {
    SW_v[swEntityCounter]->RFch = _DOC["RF_2entity"][swEntityCounter];
    init_RF(swEntityCounter);
  }
  print_sw_struct(*SW_v[swEntityCounter]);

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
          if (SW_v[x]->RFch == i)
          {
            toggleRelay(x, RF);
            lastEntry = millis();
          }
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
