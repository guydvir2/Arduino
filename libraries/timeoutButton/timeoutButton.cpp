#include <Arduino.h>
#include "timeoutButton.h"

timeoutButton::timeoutButton() : chrono(Chrono::SECONDS)
{
  _stopClock();
}
void timeoutButton::begin(uint8_t id)
{
  _useInput = false;
  _commonBegin(id);
}
void timeoutButton::begin(uint8_t pin, uint8_t trigType, uint8_t id)
{
  _useInput = true;
  Inpin = pin;
  trigTYPE = trigType;

  _commonBegin(id);
  _init_button();
}
void timeoutButton::loop()
{
  _loopClock();     /* Timeouts */
  _Button_looper(); /* Read button states */
}
bool timeoutButton::getState()
{
  return chrono.isRunning();
}
void timeoutButton::addClock(int _add, uint8_t reason)
{
  timeout += _add;

  if (!chrono.isRunning()) /* Case not ON */
  {
    startTimeout_cb(timeout, reason);
  }
  else
  {
    OPERstring.offtime = OPERstring.ontime + timeout;
    save_OperStr(OPERstring);
    newMSG = true;
  }
}
void timeoutButton::startTimeout_cb(int _TO, uint8_t reason)
{
  if (!chrono.isRunning()) /* Enter when off or at PWM different PWM value */
  {
    _TO == 0 ? timeout = defaultTimeout : timeout = _TO;
    _startClock();

    OPERstring.state = true;
    OPERstring.reason = reason;
    OPERstring.step = pressCounter;
    OPERstring.ontime = time(nullptr);
    OPERstring.offtime = OPERstring.ontime + timeout;

    save_OperStr(OPERstring);
    newMSG = true;
  }
}
void timeoutButton::stopTimeout_cb(uint8_t reason)
{
  if (chrono.isRunning())
  {
    _stopClock();
    pressCounter = 0;

    OPERstring.state = false;
    OPERstring.reason = reason;
    OPERstring.step = pressCounter;
    // OPERstring.ontime = time(nullptr);
    OPERstring.offtime = time(nullptr);
    save_OperStr(OPERstring);
    newMSG = true;
  }
}

void timeoutButton::_commonBegin(uint8_t id)
{
  Id = id;
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
  startTimeout_cb(timeout, BUTTON);
}
void timeoutButton::_ON_OFF_off_handle(Button2 &b)
{
  stopTimeout_cb(BUTTON);
}
void timeoutButton::_Momentary_handle(Button2 &b)
{
  chrono.isRunning() ? stopTimeout_cb(BUTTON) : startTimeout_cb(timeout, BUTTON);
}
void timeoutButton::_TrigSensor_handler(Button2 &b)
{
  const uint8_t update_timeout = 5; // must have passed this amount of seconds to updates timeout
  unsigned int _remaintime = remainClock();

  if (_remaintime == 0)
  {
    startTimeout_cb(timeout, BUTTON);
  }
  else
  {
    if (timeout - _remaintime > update_timeout)
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
      startTimeout_cb(0, BUTTON); /* First step - turn on the light */
    }
    else
    {
      OPERstring.step = pressCounter;
      save_OperStr(OPERstring);
      newMSG = true; /* Increase lihgt intersity */
    }
  }
  else
  {
    pressCounter = 0;
    stopTimeout_cb(BUTTON);
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
  if (chrono.isRunning())
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
  if (chrono.isRunning() && (chrono.hasPassed(timeout) || chrono.hasPassed(maxTimeout)))
  {
    stopTimeout_cb(TIMEOUT);
  }
}

void timeoutButton::print_OPERstring(oper_string &str)
{
  Serial.println("~~~~~~~ OPER_STRING START ~~~~~~~~~~~");
  Serial.print("state:\t\t\t");
  Serial.println(str.state);

  Serial.print("reason:\t\t\t");
  Serial.println(INPUT_ORG[str.reason]);

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
void timeoutButton::read_OperStr(oper_string &str)
{
  File _file = LITFS.open(_operfile, "r");
  _file.read((byte *)&str, sizeof(str));
  _file.close();
}

// /* ~~~~~~~ LightButton Class ~~~~~~~~ */
// template<uint8_t N>
// LightButton<N>::LightButton() : Button()
// {
// }
// void LightButton::begin(uint8_t id)
// {
//   _buttonID = id;
//   _init_button();
//   _init_light();
//   _init_onAtBoot();
// }
// void LightButton::_newActivity_handler()
// {
//   if (OPstr->state) /* ON */
//   {
//     if (isPWM())
//     {
//       if (Button.pressCounter <= max_pCount)
//       {
//         _turnONlights();
//         Serial.println("PWM CHNGE");
//       }
//       else
//       {
//         Button.pressCounter = 0;
//         _turnOFFlights();
//         Button.stopTimeout_cb(BUTTON);
//       }
//     }
//     else
//     {
//       Serial.println("LIGHTS_ON");
//       _turnONlights();
//     }
//   }
//   else
//   {
//     Serial.println("LIGHTS_OFF");
//     _turnOFFlights();
//   }
//   Button.print_OPERstring(*OPstr);

//   sendMSG(*OPstr);
//   Button.newMSG = false;
// }

// void LightButton::loop()
// {
//   Button.loop();
//   if (Button.newMSG)
//   {
//     _newActivity_handler();
//   }
// }
// unsigned int LightButton::remainClock()
// {
//   return Button.remainClock();
// }
// bool LightButton::getState()
// {
//   return Button.getState();
// }
// void LightButton::stopTimeout_cb(uint8_t reason)
// {
//   Button.stopTimeout_cb(reason);
// }
// void LightButton::startTimeout_cb(int _TO, uint8_t reason)
// {
//   Button.startTimeout_cb(conv2Minute(_TO), reason);
// }

// void LightButton::_init_button()
// {
//   Button.maxTimeout = conv2Minute(Button.maxTimeout);
//   Button.defaultTimeout = conv2Minute(Button.defaultTimeout);
//   Button.begin(_buttonID);                     /* Not using button */
//   Button.begin(inputPin, trigType, _buttonID); /* Using button */
// }
// void LightButton::_init_light()
// {
//   if (!outputPWM)
//   {
//     init(outputPin, output_ON); /* GPIO output */
//   }
//   else
//   {
//     init(outputPin, PWM_res, dimmablePWM); /* PWM output */
//   }
//   auxFlag(indicPin); /* init if pin != 255 */
// }
// void LightButton::_init_onAtBoot()
// {
//   if (OnatBoot)
//   {
//     startTimeout_cb(0, PWRON);
//   }
// }
// void LightButton::_turnONlights()
// {
//   // if (!is_ON())
//   // {
//   turnON(Button.pressCounter);
//   // }
// }
// void LightButton::_turnOFFlights()
// {
//   if (is_ON())
//   {
//     turnOFF();
//   }
// }