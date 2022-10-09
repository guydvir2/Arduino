#include "homeController.h"

homeCtl::homeCtl()
{
}
bool homeCtl::loop()
{
  _SW_loop();
  _Win_loop();

  if (_use_RF)
  {
    _RF_loop();
  }
  return _MSG.newMSG;
}

void homeCtl::create_SW(char *topic, uint8_t sw_type, bool is_virtual, int timeout_m, uint8_t RF_ch)
{
  SW_v[_swEntityCounter] = new smartSwitch;
  SW_v[_swEntityCounter]->set_name(topic);
  SW_v[_swEntityCounter]->set_input(_input_pins[_inIOCounter], sw_type); /* input is an option */
  SW_v[_swEntityCounter]->set_id(_swEntityCounter);
  SW_v[_swEntityCounter]->set_timeout(timeout_m);

  /* Phsycal or Virtual output ?*/
  if (!is_virtual)
  {
    SW_v[_swEntityCounter]->set_output(_output_pins[_outIOCounter]);
    _outIOCounter++;
  }
  else
  {
    SW_v[_swEntityCounter]->set_output();
  }

  /* Assign RF to SW */
  if (RF_ch != 255)
  {
    _RF_ch_2_SW[_swEntityCounter] = _RF_freq[_swEntityCounter];
    _init_RF(_swEntityCounter);
  }

  SW_v[_swEntityCounter]->get_prefences();
  _inIOCounter++;
  _swEntityCounter++;
}
void homeCtl::create_Win(char *topic, bool is_virtual, bool use_ext_sw)
{
  winSW_V[_winEntityCounter] = new WinSW;
  winSW_V[_winEntityCounter]->set_input(_input_pins[_inIOCounter], _input_pins[_inIOCounter + 1]);
  winSW_V[_winEntityCounter]->set_name(topic);

  // <<<<<<<<<<< Define input and output pins >>>>>>>>>>>>>>
  if (is_virtual) /* a virtCMD on output */
  {
    winSW_V[_winEntityCounter]->set_output(); /* empty definition --> virtCMD */
  }
  else /* Physical Switching input & output */
  {
    winSW_V[_winEntityCounter]->set_output(_output_pins[_outIOCounter], _output_pins[_outIOCounter + 1]);
    _outIOCounter += 2;
  }

  // <<<<<<<<<<< Define Ext_input pins , if needed >>>>>>>>>>>>>>
  if (use_ext_sw) /* define a Secondary input for a window */
  {
    winSW_V[_winEntityCounter]->set_ext_input(_input_pins[_inIOCounter + 2], _input_pins[_inIOCounter + 3]);
    _inIOCounter += 2;
  }

  // <<<<<<<<<<< Init instance  >>>>>>>>>>>>>>
  winSW_V[_winEntityCounter]->set_extras(); /* Timeout & lockdown */
  winSW_V[_winEntityCounter]->print_preferences();

  // <<<<<<<<< Incrementing Counters >>>>>>>>>>
  _winEntityCounter++;
  _inIOCounter += 2;
}

void homeCtl::clear_telemetryMSG()
{
  _MSG.newMSG = false;
}
void homeCtl::_SW_newMSG(uint8_t i)
{
  _MSG.id = i;
  _MSG.type = 1;
  _MSG.newMSG = true;
  _MSG.virtCMD = SW_v[i]->is_virtCMD();
  _MSG.state = SW_v[i]->telemtryMSG.state;
  _MSG.reason = SW_v[i]->telemtryMSG.reason;
}
void homeCtl::_Win_newMSG(uint8_t i)
{
  _MSG.id = i;
  _MSG.type = 0;
  _MSG.newMSG = true;
  _MSG.virtCMD = winSW_V[i]->virtCMD;
  _MSG.state = winSW_V[i]->MSG.state;
  _MSG.reason = winSW_V[i]->MSG.reason;
}

void homeCtl::_SW_loop()
{
  for (uint8_t i = 0; i < _swEntityCounter; i++)
  {
    if (SW_v[i]->loop())
    {
      _SW_newMSG(i);
      SW_v[i]->clear_newMSG();
    }
  }
}
void homeCtl::_Win_loop()
{
  for (uint8_t x = 0; x < _winEntityCounter; x++)
  {
    if (winSW_V[x]->loop())
    {
      _Win_newMSG(x);
      winSW_V[x]->clear_newMSG();
    }
  }
}
void homeCtl::_RF_loop()
{
  if (RF_v->available()) /* New transmission */
  {
    // sprintf(temp, "Received %d / %dbit Protocol: ", RFreader.getReceivedValue(), RFreader.getReceivedBitlength(), RFreader.getReceivedProtocol());
    static unsigned long lastEntry = 0;

    for (uint8_t i = 0; i < sizeof(_RF_freq) / sizeof(_RF_freq[0]); i++)
    {
      if (_RF_freq[i] == RF_v->getReceivedValue() && millis() - lastEntry > 300)
      {
        for (uint8_t x = 0; x < _swEntityCounter; x++) /* choose the right switch to the received code */
        {
          if (_RF_ch_2_SW[x] == i)
          {
            _toggle_SW_RF(x);
            lastEntry = millis();
          }
        }
      }
    }
    RF_v->resetAvailable();
  }
}

void homeCtl::_init_RF(uint8_t i)
{
  if (_RF_ch_2_SW[_swEntityCounter] != 255 && RF_v == nullptr)
  {
    _use_RF = true;
    RF_v = new RCSwitch();
    RF_v->enableReceive(_RFpin);
  }
}
void homeCtl::_toggle_SW_RF(uint8_t i)
{
  if (SW_v[i]->is_virtCMD())
  {
    if (SW_v[i]->get_SWtype() == 2) /* virtCMD + PushButton --> output state is unknown*/
    {
      char top[50];
      sprintf(top, "%s/State", SW_v[i]->name);
      // iot.mqttClient.subscribe(top);
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

void homeCtl::set_inputs(uint8_t arr[], uint8_t arr_size)
{
  for (uint8_t i = 0; i < arr_size; i++)
  {
    _input_pins[i] = arr[i];
  }
}
void homeCtl::set_outputs(uint8_t arr[], uint8_t arr_size)
{
  for (uint8_t i = 0; i < arr_size; i++)
  {
    _output_pins[i] = arr[i];
  }
}
void homeCtl::set_RFch(uint8_t arr[], uint8_t arr_size)
{
  for (uint8_t i = 0; i < arr_size; i++)
  {
    _RF_freq[i] = arr[i];
  }
}

uint8_t homeCtl::get_SW_entCounter()
{
  return _swEntityCounter;
}
uint8_t homeCtl::get_Win_entCounter()
{
  return _winEntityCounter;
}
uint8_t homeCtl::get_inputPins(uint8_t i)
{
  return _input_pins[i];
}
uint8_t homeCtl::get_outputPins(uint8_t i)
{
  return _output_pins[i];
}

void homeCtl::Win_switchCB(uint8_t i, uint8_t state)
{
  winSW_V[i]->set_WINstate(state, MQTT);
#if RETAINED_MSG
  MQTT_clear_retained(winSW_V[i]->name);
#endif
}
void homeCtl::SW_switchCB(uint8_t i, uint8_t state, unsigned int TO)
{
  if (state == 1) /* ON */
  {
    SW_v[i]->turnON_cb(EXT_0, TO);
  }
  else if (state == 0) /* OFF */
  {
    SW_v[i]->turnOFF_cb(EXT_0);
  }
#if RETAINED_MSG
  MQTT_clear_retained(SW_v[i]->name);
#endif
}