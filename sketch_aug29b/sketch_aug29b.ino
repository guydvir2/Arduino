#include <myIOT2.h>
#include <RCSwitch.h>
#include <myWindowSW.h>  /* WinSW Entities */
#include <smartSwitch.h> /* smartSwitch Entities */
#include "paramters.h"   /* Hardcoded or updated saved in flash */
#include "myIOT_settings.h"

/* ******************* Windows ******************* */
void create_WinSW_instance(JsonDocument &_DOC, uint8_t i)
{
  winSW_V[winEntityCounter] = new WinSW;

  Serial.print(" >>>>>>>>>>>>>>> Window #:");
  Serial.print(winEntityCounter);
  Serial.println(" <<<<<<<<<<<<<<<<<< ");

  // <<<<<<<<<<< Define input and output pins >>>>>>>>>>>>>>
  if (strcmp(_DOC["virtCMD"][i], "") != 0) /* a virtCMD on output */
  {
    winSW_V[winEntityCounter]->virtCMD = true;
    winSW_V[winEntityCounter]->def(inPinsArray[lastUsed_inIO], inPinsArray[lastUsed_inIO + 1]);
    strcpy(winSW_V[winEntityCounter]->name, _DOC["virtCMD"][i].as<const char *>());

    Serial.print("virtCMD :\t");
    Serial.println(winSW_V[winEntityCounter]->virtCMD);
    Serial.print("virtCMD MQTT:\t");
    Serial.println(winSW_V[winEntityCounter]->name);
    Serial.print("in_pins #:\t");
    Serial.print(inPinsArray[lastUsed_inIO]);
    Serial.print("; ");
    Serial.println(inPinsArray[lastUsed_inIO + 1]);
    Serial.println("out_pins #:\t NONE");
  }
  else /* Physical Switching input & output */
  {
    winSW_V[winEntityCounter]->virtCMD = false;
    winSW_V[winEntityCounter]->def(inPinsArray[lastUsed_inIO], inPinsArray[lastUsed_inIO + 1], outPinsArray[lastUsed_outIO], outPinsArray[lastUsed_outIO + 1]);

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
void _newMSGcb(uint8_t x)
{
  if (winSW_V[x]->newMSGflag)
  {
    _gen_WinMSG(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);       /* Generate MQTT MSG */
    _post_Win_virtCMD(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x); /* Sending MQTT cmd in case of virt Win*/
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

/* ******************* Switches  ******************* */
void create_SW_instance(JsonDocument &_DOC, uint8_t i)
{
  SW_v[swEntityCounter] = new smartSwitch;
  SW_v[swEntityCounter]->set_name((char *)_DOC["virtCMD"][i].as<const char *>());
  SW_v[swEntityCounter]->set_input(inPinsArray[lastUsed_inIO], _DOC["SW_buttonTypes"][swEntityCounter] | 1);
  SW_v[swEntityCounter]->set_id(swEntityCounter); /* MUST be after "set_input" function */

  /* Phsycal or Virtual output ?*/
  if (strcmp(_DOC["virtCMD"][i], "") == 0)
  {
    SW_v[swEntityCounter]->set_output(outPinsArray[lastUsed_outIO]);
    lastUsed_outIO++;
  }

  /* Config timeout duration to SW */
  if (_DOC["SW_timeout"][i].as<int>() > 0)
  {
    SW_v[swEntityCounter]->set_timeout(_DOC["SW_timeout"][i].as<int>());
  }

  /* Assign RF to SW */
  if (_DOC["SW_RF"][swEntityCounter] != 255)
  {
    linkRF2SW[swEntityCounter] = _DOC["SW_RF"][swEntityCounter];
    init_RF(swEntityCounter);
  }

  SW_v[swEntityCounter]->get_prefences();
  lastUsed_inIO++;
  swEntityCounter++;
}
void send_virtCMD(smartSwitch &sw)
{
  if (sw.is_virtCMD())
  {
    char msg[100];
    iot.pub_noTopic((char *)SW_MQTT_cmds[sw.telemtryMSG.state], sw.name, true);
    sprintf(msg, "[%s]: Switched [%s] Virtual [%s]", SW_Types[sw.telemtryMSG.reason], SW_MQTT_cmds[sw.telemtryMSG.state], sw.name);
    iot.pub_msg(msg);
  }
}
void SW_loop()
{
  for (uint8_t i = 0; i < swEntityCounter; i++)
  {
    if (SW_v[i]->loop())
    {
      send_virtCMD(*SW_v[i]);
      _gen_SW_MSG(i, SW_v[i]->telemtryMSG.reason, SW_v[i]->telemtryMSG.state);
      SW_v[i]->clear_newMSG();

      Serial.print("NEW_MSG: State:");
      Serial.print(SW_v[i]->telemtryMSG.state);
      Serial.print("\tReason: ");
      Serial.println(SW_v[i]->telemtryMSG.reason);
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
      SW_v[i]->turnOFF_cb(3); /* # is RF remote indetifier */
    }
    else
    {
      SW_v[i]->turnON_cb(3);
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
