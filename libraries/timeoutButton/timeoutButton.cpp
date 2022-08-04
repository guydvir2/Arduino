#include <Arduino.h>
#include "timeoutButton.h"

timeoutButton::timeoutButton() : chrono(Chrono::SECONDS)
{
  _stopWatch();
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
  _loopWatch();     /* Timeouts */
  _Button_looper(); /* Read button states */
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
    ON_cb(timeout, reason);
  }
  else
  {
    OPERstring.offtime = OPERstring.ontime + timeout;
    save_OperStr(OPERstring);
    newMSG = true;
  }
}
void timeoutButton::ON_cb(int _TO, uint8_t reason)
{
  if (!chrono.isRunning()) /* Enter when off or at PWM different PWM value */
  {
    _TO == 0 ? timeout = defaultTimeout : timeout = _TO;
    _startWatch();

    OPERstring.state = true;
    OPERstring.reason = reason;
    // if (trigTYPE == 3 && pressCounter == 0)
    // {
    // //   pressCounter =
    // }
    OPERstring.step = pressCounter;
    OPERstring.ontime = time(nullptr);
    OPERstring.offtime = OPERstring.ontime + timeout;

    save_OperStr(OPERstring);
    newMSG = true;
  }
}
void timeoutButton::OFF_cb(uint8_t reason)
{
  if (chrono.isRunning())
  {
    _stopWatch();
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
    if (timeout - _remaintime > update_timeout)
    {
      _startWatch(); /* Restart timeout after 30 sec */
    }
  }
}
void timeoutButton::_MultiPress_handler(Button2 &b)
{
  if (millis() - _lastPress < time2Repress || pressCounter == 0) //|| (_pressCounter == 0&&_lastPress))
  {
    pressCounter++;
  }
  else
  {
    pressCounter = 0;
  }
  _lastPress = millis();

  OPERstring.state = true;
  // OPERstring.reason = reason;
  OPERstring.step = pressCounter;
  // OPERstring.ontime = time(nullptr);
  // OPERstring.offtime = OPERstring.ontime + timeout;

  print_OPERstring(OPERstring);
  save_OperStr(OPERstring);
  newMSG = true;
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
    return timeout - chrono.elapsed();
  }
  else
  {
    return 0;
  }
}
void timeoutButton::_loopWatch()
{
  if (chrono.isRunning() && (chrono.hasPassed(timeout) || chrono.hasPassed(maxTimeout)))
  {
    OFF_cb(TIMEOUT);
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

/* ~~~~~~~ LightButton Class ~~~~~~~~ */
LightButton::LightButton() : Button()
{
}
void LightButton::begin(uint8_t id)
{
  _buttonID = id;
  _init_button();
  _init_light();
  _init_onAtBoot();
}
void LightButton::_newActivity_handler()
{
  if (OPstr->state) /* ON */
  {
    if (!isON() && isPWM())
    {
      Button.pressCounter = OPstr->step;
      Button.save_OperStr(*OPstr);
      Serial.println("PWM CHNGE");
    }
    else
    {
      Serial.println("LIGHTS_ON");
    }
    _turnONlights();
  }
  else
  {
    Serial.println("LIGHTS_OFF");
    _turnOFFlights();
  }
  Button.print_OPERstring(*OPstr);

  sendMSG(*OPstr);
  Button.newMSG = false;
}

void LightButton::loop()
{
  Button.loop();
  if (Button.newMSG)
  {
    _newActivity_handler();
  }
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
  Button.ON_cb(conv2Minute(_TO), reason);
}

void LightButton::_init_button()
{
  Button.maxTimeout = conv2Minute(Button.maxTimeout);
  Button.defaultTimeout = conv2Minute(Button.defaultTimeout);
  Button.begin(_buttonID);                     /* Not using button */
  Button.begin(inputPin, trigType, _buttonID); /* Using button */
}
void LightButton::_init_light()
{
  if (!outputPWM)
  {
    init(outputPin, output_ON); /* GPIO output */
  }
  else
  {
    init(outputPin, PWM_res, dimmablePWM); /* PWM output */
  }
  auxFlag(indicPin); /* init if pin != 255 */
}
void LightButton::_init_onAtBoot()
{
  if (OnatBoot)
  {
    ON_cb(0, PWRON);
  }
}
void LightButton::_turnONlights()
{
  if (!isON())
  {
    turnON(Button.pressCounter);
  }
}
void LightButton::_turnOFFlights()
{
  if (isON())
  {
    turnOFF();
  }
}