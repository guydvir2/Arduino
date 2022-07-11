#include <Arduino.h>
#include "timeoutButton.h"

timeoutButton::timeoutButton() : chrono(Chrono::SECONDS)
{
  _stopWatch();
}
void timeoutButton::begin()
{
  useInput = false;
  _init_chrono();
}
void timeoutButton::begin(uint8_t pin, uint8_t trigType, uint8_t id)
{
  useInput = true;
  _pin = pin;
  _trigType = trigType;
  _id = id;

  _init_button();
  _init_chrono();
}
void timeoutButton::loop()
{
  _loopWatch();
  _Button_looper();
}
bool timeoutButton::getState()
{
  return chrono.isRunning();
}
void timeoutButton::addWatch(int _add, uint8_t reason)
{
  timeout += _add;

  if (!chrono.isRunning()) /* Case not ON */
  {
    ON_cb(timeout, reason); //, trigger);
  }
}
void timeoutButton::ON_cb(int _TO, uint8_t reason) //, const char *trigger, uint8_t _PWMstep)
{
  if (!chrono.isRunning()) /* Enter when off or at PWM different PWM value */
  {
    _TO == 0 ? timeout = defaultTimeout : timeout = _TO;
    _startWatch();
    _extOn_cb(reason);
  }
}
void timeoutButton::OFF_cb(uint8_t reason) // const char *trigger)
{
  if (chrono.isRunning())
  {
    _extOff_cb(reason);
    _stopWatch();
  }
}
void timeoutButton::ExtON_cb(cb_func func)
{
  _extOn_cb = func;
}
void timeoutButton::ExtOFF_cb(cb_func func)
{
  _extOff_cb = func;
}
void timeoutButton::ExtMultiPress_cb(cb_func func)
{
  _extMultipress_cb = func;
}

void timeoutButton::_init_button()
{
  if (useInput)
  {
    button.begin(_pin);
    button.setID(_id);

    if (_trigType == MOMENTARY)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_Momentary_handle, this, std::placeholders::_1));
    }
    else if (_trigType == ON_OFF)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_ON_OFF_on_handle, this, std::placeholders::_1));
      button.setReleasedHandler(std::bind(&timeoutButton::_ON_OFF_off_handle, this, std::placeholders::_1));
    }
    else if (_trigType == TRIGGER_SERNSOR)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_TrigSensor_handler, this, std::placeholders::_1));
    }
    else if (_trigType == MULTI_PRESS)
    {
      button.setPressedHandler(std::bind(&timeoutButton::_MultiPress_handler, this, std::placeholders::_1));
    }
  }
}
void timeoutButton::_ON_OFF_on_handle(Button2 &b)
{
  ON_cb(timeout, BUTTON);
}
void timeoutButton::_ON_OFF_off_handle(Button2 &b)
{
  OFF_cb(BUTTON);
}
void timeoutButton::_Momentary_handle(Button2 &b)
{
  chrono.isRunning() ? OFF_cb(BUTTON) : ON_cb(timeout, BUTTON);
}
void timeoutButton::_TrigSensor_handler(Button2 &b)
{
  const uint8_t update_timeout = 5; // must have passed this amount of seconds to updates timeout
  unsigned int _remaintime = remainWatch();

  if (_remaintime == 0)
  {
    ON_cb(timeout, BUTTON);
  }
  else
  {
    if (conv2Minute(timeout) - _remaintime > update_timeout)
    {
      _startWatch(); /* Restart timeout after 30 sec */
    }
  }
}
void timeoutButton::_MultiPress_handler(Button2 &b)
{
  if (millis() - _lastPress < time2Repress || _pressCounter == 0) //|| (_pressCounter == 0&&_lastPress))
  {
    _pressCounter++;
  }
  else
  {
    _pressCounter = 0;
  }
  _lastPress = millis();
  _extMultipress_cb(_pressCounter);
}
void timeoutButton::_Button_looper()
{
  if (useInput)
  {
    button.loop();
  }
}
void timeoutButton::_init_chrono()
{
  _stopWatch();
}
void timeoutButton::_stopWatch()
{
  chrono.restart();
  chrono.stop();
  timeout = 0;
}
void timeoutButton::_startWatch()
{
  chrono.restart();
}
unsigned int timeoutButton::remainWatch()
{
  if (chrono.isRunning())
  {
    return conv2Minute(timeout) - chrono.elapsed();
  }
  else
  {
    return 0;
  }
}

void timeoutButton::_loopWatch()
{
  if (chrono.isRunning() && (chrono.hasPassed(conv2Minute(timeout)) || chrono.hasPassed(conv2Minute(maxTimeout))))
  {
    OFF_cb(TIMEOUT);
  }
}
