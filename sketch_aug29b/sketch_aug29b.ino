#include <myIOT2.h>
#include <RCSwitch.h>
#include <myWindowSW.h>  /* WinSW Entities */
#include <smartSwitch.h> /* smartSwitch Entities */
#include "paramters.h"   /* Hardcoded or updated saved in flash */
#include "dbgMode.h"
#include "myIOT_settings.h"

/* ******************* Windows ******************* */
void create_WinSW_instance(JsonDocument &_DOC, uint8_t i)
{
  winSW_V[winEntityCounter] = new WinSW;
  winSW_V[winEntityCounter]->set_input(inPinsArray[lastUsed_inIO], inPinsArray[lastUsed_inIO + 1]);
  winSW_V[winEntityCounter]->set_name(_DOC["virtCMD"][i].as<const char *>());

  // <<<<<<<<<<< Define input and output pins >>>>>>>>>>>>>>
  if (strcmp(_DOC["virtCMD"][i], "") != 0) /* a virtCMD on output */
  {
    winSW_V[winEntityCounter]->set_output(); /* empty definition --> virtCMD */
  }
  else /* Physical Switching input & output */
  {
    winSW_V[winEntityCounter]->set_output(outPinsArray[lastUsed_outIO], outPinsArray[lastUsed_outIO + 1]);
    lastUsed_outIO += 2;
  }

  // <<<<<<<<<<< Define Ext_input pins , if needed >>>>>>>>>>>>>>
  if (_DOC["WextInputs"][i] == 1) /* define a Secondary input for a window */
  {
    winSW_V[winEntityCounter]->set_ext_input(inPinsArray[lastUsed_inIO + 2], inPinsArray[lastUsed_inIO + 3]);
    lastUsed_inIO += 2;
  }

  // <<<<<<<<<<< Init instance  >>>>>>>>>>>>>>
  winSW_V[winEntityCounter]->set_extras(); /* Timeout & lockdown */
  winSW_V[winEntityCounter]->print_preferences();

  // <<<<<<<<< Incrementing Counters >>>>>>>>>>
  winEntityCounter++;
  lastUsed_inIO += 2;
}
void Win_newMsg_cb(uint8_t x)
{
  if (winSW_V[x]->virtCMD)
  {
    Win_send_virtCMD(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x); /* Sending MQTT cmd in case of virt Win*/
  }
  else
  {
    Win_send_MQTT_switch(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x); /* Generate MQTT MSG */
  }
}
void loop_WinSW()
{
  for (uint8_t x = 0; x < winEntityCounter; x++)
  {
    if (winSW_V[x]->loop())
    {
      Win_newMsg_cb(x);
      winSW_V[x]->clear_newMSG();
    }
  }
}
/* ************************************************ */

/* ******************* Switches  ******************* */
void create_SW_instance(JsonDocument &_DOC, uint8_t i)
{
  SW_v[swEntityCounter] = new smartSwitch;
  SW_v[swEntityCounter]->set_name((char *)_DOC["virtCMD"][i].as<const char *>());
  SW_v[swEntityCounter]->set_input(inPinsArray[lastUsed_inIO], _DOC["SW_buttonTypes"][swEntityCounter] | 1); /* input is an option */
  SW_v[swEntityCounter]->set_id(swEntityCounter);                                                            /* MUST be after "set_input" function */

  /* Phsycal or Virtual output ?*/
  if (strcmp(_DOC["virtCMD"][i], "") == 0)
  {
    SW_v[swEntityCounter]->set_output(outPinsArray[lastUsed_outIO]);
    lastUsed_outIO++;
  }
  else
  {
    SW_v[swEntityCounter]->set_output();
  }

  /* Config timeout duration to SW */
  if (_DOC["SW_timeout"][swEntityCounter].as<int>() > 0 | 0)
  {
    SW_v[swEntityCounter]->set_timeout(_DOC["SW_timeout"][swEntityCounter].as<int>());
  }

  /* Assign RF to SW */
  if (_DOC["SW_RF"][swEntityCounter] != 255)
  {
    linkRF2SW[swEntityCounter] = _DOC["SW_RF"][swEntityCounter].as<int>();
    init_RF(swEntityCounter);
  }

  SW_v[swEntityCounter]->get_prefences();
  lastUsed_inIO++;
  swEntityCounter++;
}
void SW_newMsg_cb(uint8_t i)
{
  if (SW_v[i]->is_virtCMD())
  {
    SW_send_virtCMD(*SW_v[i]);
  }
  else
  {
    SW_send_MQTT_switch(i, SW_v[i]->telemtryMSG.reason, SW_v[i]->telemtryMSG.state);
  }
}
void SW_loop()
{
  for (uint8_t i = 0; i < swEntityCounter; i++)
  {
    if (SW_v[i]->loop())
    {
      SW_newMsg_cb(i);
      SW_v[i]->clear_newMSG();
    }
  }
}
/* ************************************************ */

/* ******************* RF 433Hz ******************* */
RCSwitch *RF_v = nullptr;

void init_RF(uint8_t i)
{
  if (linkRF2SW[swEntityCounter] != 255 && RF_v == nullptr)
  {
    RF_v = new RCSwitch();
    RF_v->enableReceive(RFpin);
  }
}
void toggleSW_RF(uint8_t i)
{
  if (SW_v[i]->is_virtCMD())
  {
    if (SW_v[i]->get_SWtype() == 2) /* virtCMD + PushButton --> output state is unknown*/
    {
      char top[50];
      sprintf(top, "%s/State", SW_v[i]->name);
      iot.mqttClient.subscribe(top);
    }
    else
    {
    }
  }
  else
  {
    if (SW_v[i]->get_SWstate()) /* is output SW on ?*/
    {
      SW_v[i]->turnOFF_cb(EXT_1); /* # is RF remote indetifier */
    }
    else
    {
      SW_v[i]->turnON_cb(EXT_1);
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
      if (RF_keyboardCode[i] == RF_v->getReceivedValue() && millis() - lastEntry > 300)
      {
        for (uint8_t x = 0; x < swEntityCounter; x++) /* choose the right switch to the received code */
        {
          if (linkRF2SW[x] == i)
          {
            toggleSW_RF(x);
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
#if DBG_MODE
  Serial.begin(115200);
  delay(100);
#endif

  startIOTservices();
}
void loop()
{
  if (paramLoadedOK)
  {
    loop_RF();
    loop_WinSW();
    SW_loop();
  }
  iot.looper();
}
