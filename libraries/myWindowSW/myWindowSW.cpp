#include "myWindowSW.h"

WinSW::WinSW()
{
  _id = _next_id++;
}
bool WinSW::loop()
{
  _readSW();
  _timeout_looper();
  return newMSGflag;
}
void WinSW::set_id(uint8_t i)
{
  _id = i;
}
void WinSW::set_name(const char *_name)
{
  strlcpy(name, _name, MAX_NAME_LEN);
}
void WinSW::set_input(uint8_t upin, uint8_t dpin)
{
  // _windowSwitch.pin0 = upin;
  // _windowSwitch.pin1 = dpin;
  // _windowSwitch.buttonType = 2;
  // _windowSwitch.start();
  _mainSW.set_input(upin, dpin);
}
void WinSW::set_output(uint8_t outup_pin, uint8_t outdown_pin)
{
  if (outup_pin != UNDEF_INPUT && outdown_pin != UNDEF_INPUT)
  {
    outpins[0] = outup_pin;
    outpins[1] = outdown_pin;
    pinMode(outpins[0], OUTPUT);
    pinMode(outpins[1], OUTPUT);
    virtCMD = false;
  }
  else
  {
    virtCMD = true;
  }
  _allOff();
}
void WinSW::set_ext_input(uint8_t upin, uint8_t dpin)
{
  if (upin != UNDEF_INPUT && dpin != UNDEF_INPUT)
  {
    useExtSW = true;
    // _windowSwitch_ext.pin0 = upin;
    // _windowSwitch_ext.pin1 = dpin;
    // _windowSwitch_ext.buttonType = 2;
    // _windowSwitch_ext.start();
    _extSW.set_input(upin, dpin);
  }
}
void WinSW::set_WINstate(uint8_t state, uint8_t reason) /* External Callback */
{
  _switch_cb(state, reason);
}
void WinSW::set_extras(bool useLockdown, int timeout_clk)
{
  _uselockdown = useLockdown;
  _timeout_clk = timeout_clk;
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
void WinSW::print_preferences()
{
  Serial.print(" >>>>>> Window #:");
  Serial.print(_id);
  Serial.println(" <<<<<< ");

  Serial.print("Output :\t");
  Serial.println(virtCMD ? "Virutal" : "Relay");

  Serial.print("MQTT:\t");
  Serial.println(name);

  Serial.print("in_pins #:\t");
  // Serial.print(_windowSwitch.pin0);
  Serial.print(_mainSW.get_pins(0));
  Serial.print("; ");
  // Serial.println(_windowSwitch.pin1);
  Serial.print(_mainSW.get_pins(1));

  Serial.print("out_pins #:\t");
  Serial.print(outpins[0]);
  Serial.print("; ");
  Serial.println(outpins[1]);

  Serial.print("ext_pins #:\t");
  // Serial.print(_windowSwitch_ext.pin0);
  Serial.print(_extSW.get_pins(0));

  Serial.print("; ");
  // Serial.println(_windowSwitch_ext.pin1);
  Serial.print(_extSW.get_pins(1));

  Serial.print("use timeout:\t");
  Serial.println(_timeout_clk);

  Serial.print("use lockdown:\t");
  Serial.println(_uselockdown ? "YES" : "NO");

  Serial.println(" >>>>>>>> END <<<<<<<< ");
}
void WinSW::clear_newMSG()
{
  newMSGflag = false;
}
uint8_t WinSW::get_id()
{
  return _id;
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
  if (((_uselockdown && _lockdownState == false) || _uselockdown == false) && (state != get_winState() || virtCMD == true))
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
  uint8_t switchRead = _mainSW.read(); //_windowSwitch.read(); /*  0: stop; 1: up; 2: down; 3:err ; 4: nochange*/
  if (switchRead < 3)
  {
    _switch_cb(switchRead, BUTTON);
    return;
  }
  if (useExtSW)
  {
    switchRead = _mainSW.read(); //_windowSwitch_ext.read(); /*  0: stop; 1: up; 2: down; 3:err ; 4: nochange*/
    if (switchRead < 3)
    {
      _switch_cb(switchRead, BUTTON2);
      return;
    }
  }
}
void WinSW::_timeout_looper()
{
  if (_timeout_clk > 0 && _timeoutcounter > 0)
  {
    if (millis() - _timeoutcounter > _timeout_clk * 1000)
    {
      _switch_cb(STOP, TIMEOUT);
    }
  }
}

RockerSW::RockerSW()
{
}
uint8_t RockerSW::get_pins(uint8_t i)
{
  return _pins[i];
}
void RockerSW::set_input(uint8_t upPin, uint8_t downPin, uint8_t active_dir)
{
  _pins[0] = upPin;
  _pins[1] = downPin;

  if (active_dir == INPUT_PULLUP)
  {
    pinMode(_pins[0], INPUT_PULLUP);
    pinMode(_pins[1], INPUT_PULLUP);
  }
  else if (active_dir == INPUT)
  {
    pinMode(_pins[0], INPUT);
    pinMode(_pins[1], INPUT);
  }
  _lastPins_read[0] = digitalRead(_pins[0]);
  _lastPins_read[1] = digitalRead(_pins[1]);
}
uint8_t RockerSW::get_raw()
{
  bool cur_pin0 = digitalRead(_pins[0]);
  bool cur_pin1 = digitalRead(_pins[1]);

  if (cur_pin0 == !PRESSED && cur_pin1 == !PRESSED)
  {
    return STATE_OFF;
  }
  else if (cur_pin0 == PRESSED && cur_pin1 == !PRESSED)
  {
    return STATE_1;
  }
  else if (cur_pin0 == !PRESSED && cur_pin1 == PRESSED)
  {
    return STATE_2;
  }
  else
  {
    return STATE_ERR;
  }
}
uint8_t RockerSW::read()
{
  /*
  Return codes:
  0 - Both are off
  1 - Pin0 is Pressed
  2 - Pin1 is Pressed
  3 - no change
  4 - err
  */

  bool cur_pin0 = digitalRead(_pins[0]);
  bool cur_pin1 = digitalRead(_pins[1]);

  if (cur_pin0 == _lastPins_read[0] && cur_pin1 == _lastPins_read[1]) /* no-change */
  {
    return 3;
  }
  else
  {
    delay(DEBOUNCE_MS);
    bool cur2_pin0 = digitalRead(_pins[0]);
    bool cur2_pin1 = digitalRead(_pins[1]);

    if (cur2_pin0 != cur_pin0 || cur2_pin1 != cur2_pin1)
    {
      return STATE_ERR;
    }
    else if (cur2_pin0 == cur_pin0 && _lastPins_read[0] != cur_pin0)
    {
      _lastPins_read[0] = cur_pin0;
      return cur2_pin0 == PRESSED ? STATE_1 : STATE_OFF;
    }
    else if (cur2_pin1 == cur_pin1 && _lastPins_read[1] != cur_pin1)
    {
      _lastPins_read[1] = cur_pin1;
      return cur2_pin1 == PRESSED ? STATE_2 : STATE_OFF;
    }
    else
    {
      return STATE_ERR;
    }
  }
}