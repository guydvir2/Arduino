#include <Arduino.h>
#include "timeoutButton.h"

timeoutButton::timeoutButton() : chrono(Chrono::SECONDS)
{
  _stopClock();
}
void timeoutButton::begin(uint8_t id, bool useSave, bool useTO)
{
  _useInput = false;
  _commonBegin(id, useSave, useTO);
}
void timeoutButton::begin(uint8_t pin, uint8_t trigType, uint8_t id, bool useSave, bool useTO)
{
  _useInput = true;
  Inpin = pin;
  trigTYPE = trigType;

  _commonBegin(id, useSave, useTO);
  _init_button();
}
void timeoutButton::loop()
{
  _loopClock();     /* Timeouts */
  _Button_looper(); /* Read button states */
}
bool timeoutButton::getState()
{
  if (_useTimeout)
  {
    return chrono.isRunning();
  }
  else
  {
    return _isButtonPressed;
  }
}
bool timeoutButton::get_useSave()
{
  return _useSave;
}
bool timeoutButton::get_useTimeout()
{
  return _useTimeout;
}
bool timeoutButton::get_useInput()
{
  return _useInput;
}
void timeoutButton::addClock(int _add, uint8_t reason)
{
  // use timeout ?
  timeout += _add;

  if (!getState()) /* Case not ON */
  {
    Button_pressON(timeout, reason);
  }
  else
  {
    updateOperStr(OPERstring, true, reason, OPERstring.step, OPERstring.ontime, OPERstring.ontime + timeout);
    /* Add message ?*/
  }
}
void timeoutButton::Button_pressON(int _TO, uint8_t reason)
{
  if (!getState()) /* Enter when off or at PWM different PWM value */
  {
    _TO == 0 ? timeout = defaultTimeout : timeout = _TO;
    _startClock();

    updateOperStr(OPERstring, true, reason, pressCounter, time(nullptr), time(nullptr) + timeout);
    flag2ON = true;
    _isButtonPressed = true;
  }
}
void timeoutButton::Button_pressOFF(uint8_t reason)
{
  if (getState())
  {
    _stopClock();
    pressCounter = 0;
    updateOperStr(OPERstring, false, reason, pressCounter, OPERstring.ontime, time(nullptr));
    flag2OFF = true;
    _isButtonPressed = false;
  }
}

void timeoutButton::_commonBegin(uint8_t id, bool useSave, bool useTO)
{
  Id = id;
  _useSave = useSave;
  _useTimeout = useTO;
  sprintf(_operfile, "/opfile%d.txt", Id);
  _init_chrono();
}
void timeoutButton::_init_button()
{
  if (_useInput)
  {
    button.begin(Inpin);
    button.setID(Id);

    if (trigTYPE == MOMENTARY)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_Momentary_handle, this, std::placeholders::_1));
    }
    else if (trigTYPE == ON_OFF)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_ON_OFF_on_handle, this, std::placeholders::_1));
      button.setReleasedHandler(std::bind(&timeoutButton::_ON_OFF_off_handle, this, std::placeholders::_1));
    }
    else if (trigTYPE == TRIGGER_SERNSOR)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_TrigSensor_handler, this, std::placeholders::_1));
    }
    else if (trigTYPE == MULTI_PRESS)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_MultiPress_handler, this, std::placeholders::_1));
    }
  }
}
void timeoutButton::_ON_OFF_on_handle(Button2 &b)
{
  Button_pressON(timeout, BUTTON);
}
void timeoutButton::_ON_OFF_off_handle(Button2 &b)
{
  Button_pressOFF(BUTTON);
}
void timeoutButton::_Momentary_handle(Button2 &b)
{
  getState() ? Button_pressOFF(BUTTON) : Button_pressON(timeout, BUTTON);
}
void timeoutButton::_TrigSensor_handler(Button2 &b)
{
  const uint8_t update_timeout = 5; // must have passed this amount of seconds to restart timeout
  unsigned int _remaintime = remainClock();

  if (_remaintime == 0)
  {
    Button_pressON(timeout, BUTTON); /* First detection turns ON */
  }
  else
  {
    if (timeout - _remaintime > update_timeout) /* NOT DEFINED RIGHT. CHECK LATER */
    {
      _startClock(); /* Restart timeout after 30 sec */
    }
  }
}
void timeoutButton::_MultiPress_handler(Button2 &b)
{
  if (millis() - _lastPress < time2Repress || pressCounter == 0)
  {
    pressCounter++;
    if (pressCounter == 1)
    {
      Button_pressON(0, BUTTON); /* First step - turn on the light */
    }
    else
    {
      /* Press update */
      updateOperStr(OPERstring, true, OPERstring.reason, pressCounter, OPERstring.ontime, OPERstring.offtime);
      flag2ON = true; /* notify the update */
    }
  }
  else
  {
    Button_pressOFF(BUTTON);
  }
  _lastPress = millis();
}
void timeoutButton::_Button_looper()
{
  if (_useInput)
  {
    button.loop();
  }
}
void timeoutButton::_init_chrono()
{
  _stopClock();
}
void timeoutButton::_stopClock()
{
  chrono.restart();
  chrono.stop();
  timeout = 0;
}
void timeoutButton::_startClock()
{
  chrono.restart();
}
unsigned int timeoutButton::remainClock()
{
  if (getState())
  {
    return timeout - chrono.elapsed();
  }
  else
  {
    return 0;
  }
}
void timeoutButton::_loopClock()
{
  if (_useTimeout && chrono.isRunning() && (chrono.hasPassed(timeout) || chrono.hasPassed(maxTimeout)))
  {
    Button_pressOFF(TIMEOUT);
  }
}

void timeoutButton::print_OPERstring(oper_string &str)
{
  Serial.print("~~~~~~~ OPER_STRING START #");
  Serial.print(Id);
  Serial.println(" ~~~~~~~~~");

  Serial.print("state:\t\t\t");
  Serial.println(str.state);

  Serial.print("reason:\t\t\t");
  Serial.println(str.reason);

  Serial.print("step:\t\t\t");
  Serial.println(str.step);

  Serial.print("ontime:\t\t\t");
  Serial.println(str.ontime);

  Serial.print("offtime:\t\t");
  Serial.println(str.offtime);
  Serial.println("~~~~~~~ OPER_STRING END ~~~~~~~~~~~~~");
}
void timeoutButton::save_OperStr(oper_string &str)
{
  File _file = LITFS.open(_operfile, "w");
  _file.write((byte *)&str, sizeof(str));
  _file.close();
}
bool timeoutButton::read_OperStr(oper_string &str)
{
  File _file = LITFS.open(_operfile, "r");
  if (!_file)
  {
    _file.close();
    return 0;
  }
  else
  {
    _file.read((byte *)&str, sizeof(str));
    _file.close();
    return 1;
  }
}
void timeoutButton::updateOperStr(oper_string &str, bool state, uint8_t reason, uint8_t step, time_t ontime, time_t offtime)
{
  str.state = state;
  str.reason = reason;
  str.step = step;
  str.ontime = ontime;
  str.offtime = offtime;
  if (_useSave)
  {
    save_OperStr(str);
  }
}