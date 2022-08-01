#include <Arduino.h>
#include "timeoutButton.h"

timeoutButton::timeoutButton() : chrono(Chrono::SECONDS)
{
  _stopWatch();
}
void timeoutButton::begin(uint8_t id)
{
  _id = id;
  _useInput = false;
  _init_chrono();
}
void timeoutButton::begin(uint8_t pin, uint8_t trigType, uint8_t id)
{
  _useInput = true;
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
bool timeoutButton::ON_cb(int _TO, uint8_t reason) 
{
  if (!chrono.isRunning()) /* Enter when off or at PWM different PWM value */
  {
    _TO == 0 ? timeout = defaultTimeout : timeout = _TO;
    _startWatch();
    lightsON(reason);
    return 1;
  }
  else
  {
    return 0;
  }
}
bool timeoutButton::OFF_cb(uint8_t reason) 
{
  if (chrono.isRunning())
  {
    _stopWatch();
    lightsOFF();
    return 1;
  }
  else
  {
    return 0;
  }
}

void timeoutButton::set_lights(TurnOnLights *light)
{
  Light = light;
}
bool timeoutButton::lightsON(uint8_t step)
{
  return Light->turnON(step);
}
bool timeoutButton::lightsOFF()
{
  return Light->turnOFF();
}
bool timeoutButton::lightsPWM(int val)
{
  return Light->PWMvalue(val);
}

void timeoutButton::_init_button()
{
  if (_useInput)
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
  // _extMultipress_cb(_pressCounter, _id);
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

/* ~~~~~~~ LightButton Class ~~~~~~~~ */
LightButton::LightButton() : Button(), Light()
{
}
bool LightButton::isON()
{
  return Light.isON();
}
void LightButton::blink(uint8_t blinks, int _delay)
{
  Light.blink(blinks, _delay);
}

void LightButton::begin(uint8_t id)
{
  _buttonID = id;
  Button.begin(_buttonID);                     /* Not using button */
  Button.begin(inputPin, trigType, _buttonID); /* Using button */
  Button.set_lights(&Light);
  
  Button.maxTimeout = maxON_minutes;
  Button.defaultTimeout = def_TO_minutes;

  Light.init(outputPin, output_ON);            /* GPIO output */
  Light.init(outputPin, PWM_res, dimmablePWM); /* PWM output */
  Light.auxFlag(indicPin);                     /* init if pin != 255 */

  Light.defStep = defPWM;
  Light.limitPWM = limitPWM;
  Light.maxSteps = max_pCount;
}
void LightButton::loop()
{
  Button.loop();
}
unsigned int LightButton::remainWatch()
{
  return Button.remainWatch();
}

bool LightButton::getState()
{
  return Button.getState();
}
void LightButton::OFF_cb(uint8_t reason)
{
  Button.OFF_cb(reason);
}
void LightButton::ON_cb(int _TO, uint8_t reason)
{
  Button.ON_cb(_TO, reason);
}
