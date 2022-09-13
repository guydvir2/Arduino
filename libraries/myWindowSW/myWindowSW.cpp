#include "myWindowSW.h"

WinSW::WinSW()
{
}
void WinSW::def(uint8_t upin, uint8_t dpin, uint8_t outup_pin, uint8_t outdown_pin)
{
  outpins[0] = outup_pin;
  outpins[1] = outdown_pin;

  _windowSwitch.pin0 = upin;
  _windowSwitch.pin1 = dpin;
  _windowSwitch.buttonType = 2;
}
void WinSW::def_extSW(uint8_t upin, uint8_t dpin)
{
  if (upin != 255)
  {
    _useExtSW = true;
  }
  _windowSwitch_ext.pin0 = upin;
  _windowSwitch_ext.pin1 = dpin;
  _windowSwitch_ext.buttonType = 2;
}
void WinSW::def_extras(bool useTimeout, bool useLockdown, int timeout_clk)
{
  _useTimeout = useTimeout;
  _uselockdown = useLockdown;
  _timeout_clk = timeout_clk;
}
void WinSW::start()
{
  _windowSwitch.start();
  if (_useExtSW)
  {
    _windowSwitch_ext.start();
  }

  if (outpins[0] != 255)
  {
    pinMode(outpins[0], OUTPUT);
    pinMode(outpins[1], OUTPUT);
  }
  else
  {
    virtCMD = true; /* Not switching Relays */
  }
  _allOff();
  id = _next_id++;
}
void WinSW::init_lockdown()
{
  if (_uselockdown && _lockdownState == false)
  {
    _switch_cb(DOWN, LCKDOWN);
    _lockdownState = true;
  }
}
void WinSW::release_lockdown()
{
  if (_uselockdown && _lockdownState == true)
  {
    _lockdownState = false;
  }
}
void WinSW::loop()
{
  _readSW();
  _timeout_looper();
}
void WinSW::ext_SW(uint8_t state, uint8_t reason) /* External Callback */
{
  _switch_cb(state, reason);
}
uint8_t WinSW::get_winState()
{
  bool uprel = digitalRead(outpins[0]);
  bool downrel = digitalRead(outpins[1]);

  if (downrel == !RELAY_ON && uprel == !RELAY_ON)
  {
    return STOP;
  }
  else if (downrel == RELAY_ON && uprel == !RELAY_ON)
  {
    return DOWN;
  }
  else if (downrel == !RELAY_ON && uprel == RELAY_ON)
  {
    return UP;
  }
  else
  {
    return ERR;
  }
}
uint8_t WinSW::get_id()
{
  return id;
}
void WinSW::set_id(uint8_t i)
{
  id = i;
}
uint8_t WinSW::_next_id = 0;
void WinSW::_allOff()
{
  if (!virtCMD)
  {
    digitalWrite(outpins[0], !RELAY_ON);
    digitalWrite(outpins[1], !RELAY_ON);
    delay(10);
  }
}
void WinSW::_winUP()
{
  _allOff();
  winUP;
}
void WinSW::_winDOWN()
{
  _allOff();
  winDOWN;
}
void WinSW::_switch_cb(uint8_t state, uint8_t i)
{
  if (((_uselockdown && _lockdownState == false) || _uselockdown == false) && state != get_winState())
  {
    if (state == STOP)
    {
      _allOff();
      newMSGflag = true;
      _timeoutcounter = 0;
    }
    else if (state == UP)
    {
      _winUP();
      newMSGflag = true;
      _timeoutcounter = millis();
    }
    else if (state == DOWN)
    {
      _winDOWN();
      newMSGflag = true;
      _timeoutcounter = millis();
    }
    else
    {
      MSG.state = ERR;
      MSG.reason = i;
      return;
    }
    MSG.state = state;
    MSG.reason = i;
  }
}
void WinSW::_readSW()
{
  uint8_t switchRead = _windowSwitch.read(); /*  0: stop; 1: up; 2: down; 3:err ; 4: nochange*/
  if (switchRead < 3)
  {
    _switch_cb(switchRead, BUTTON);
    return;
  }
  if (_useExtSW)
  {
    switchRead = _windowSwitch_ext.read(); /*  0: stop; 1: up; 2: down; 3:err ; 4: nochange*/
    if (switchRead < 3)
    {
      _switch_cb(switchRead, BUTTON2);
      return;
    }
  }
}
void WinSW::_timeout_looper()
{
  if (_useTimeout && _timeoutcounter > 0)
  {
    if (millis() - _timeoutcounter > _timeout_clk * 1000)
    {
      _switch_cb(STOP, TIMEOUT);
    }
  }
}
