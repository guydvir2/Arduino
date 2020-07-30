#include "Arduino.h"
#include "myPIR.h"

PIRsensor::PIRsensor(int Pin, char *nick, int logic_length, bool detect)
{
  _pin = Pin;
  sensNick = nick;
  _length_logic_state = logic_length;
  _isDetect = detect;
}
void PIRsensor::start()
{
  pinMode(_pin, INPUT);
  digitalWrite(_pin, !_isDetect);
}
void PIRsensor::detect_cb(cb_func cb)
{
  _detect_cb = cb;
  _use_detfunc = true;
}
void PIRsensor::end_detect_cb(cb_func cb)
{
  _end_detect_cb = cb;
  _use_enddetfunc = true;
}
void PIRsensor::update_timer_end()
{
  _endTimer = millis() + (long)timer_duration * 1000;
}
void PIRsensor::check_timer()
{
  if (use_timer && _timer_is_on) // check if timer is on and time remains
  {
    if (millis() > _endTimer)
    {
      // ending Timer
      _timer_is_on = false;
      timeLeft = 0;
      if (_use_enddetfunc)
      {
        _end_detect_cb();
      }
    }
    else
    {
      // Timer remains
      timeLeft = (int)((_endTimer - millis()) / 1000);
    }
  }
}
void PIRsensor::detection_callback()
{
  detCounts++;
  _lastState = true;
  _lastDetection_clock = millis();

  if (use_timer)
  {
    if (_timer_is_on == false)
    {
      update_timer_end();
      _timer_is_on = true;
    }
    else
    {
      if (trigger_once == false)
      {
        update_timer_end();
      }
    }
  }
  if (use_serial)
  {
    // Serial.print("~~detect ");
    // Serial.println(sensNick);
  }
  if (_use_detfunc)
  {
    _detect_cb();
  }
}
void PIRsensor::end_detection_callback()
{
  _lastState = false;

  if (use_serial)
  {
    // Serial.print("~~end_detect ");
    // Serial.println(sensNick);
  }
}
void PIRsensor::checkSensor()
{
  bool detect_clock_ok = millis() > _lastDetection_clock + (long)((ignore_det_interval + _length_logic_state) * 1000); // timeout - minimal time between detections

  logic_state = millis() <= _lastDetection_clock + (long)(_length_logic_state * 1000); // logic flag for sensor to be indetection mode
  sens_state = digitalRead(_pin);                                                      // physical sensor read

  if (logic_state == false)
  {
    if (sens_state == _isDetect && _lastState == false)
    {
      if (detect_clock_ok || _lastDetection_clock == 0)

      {
        detection_callback();
      }
    }
    else if (sens_state != _isDetect && _lastState)
    {
      end_detection_callback();
    }
  }
}
void PIRsensor::looper()
{
  if (stop_sensor == false)
  {
    check_timer();
    checkSensor();
  }
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

SensorSwitch::SensorSwitch(byte sensorPin, byte switchPin, int timeout_mins, byte extPin)
{
  _sensorPin = sensorPin;
  _switchPin = switchPin;
  _extPin = extPin;
  _timeout_mins = timeout_mins;
  _stored_timeout = _timeout_mins;
}
void SensorSwitch::turnOff()
{
  if (usePWM)
  {
    for (int i = _currentPWMval; i >= 0; i = i - _PWMdimm_step)
    {
      analogWrite(_switchPin, i);
      delay(_PWMdimm_delay);
    }
  }
  else
  {
    digitalWrite(_switchPin, !RelayON_def);
  }
  swState = 0.0;
  _ONclock = 0;
}
void SensorSwitch::turnOn(int TO)
{
  if (usePWM)
  {
    if (_currentPWMval == 0)
    {
      _currentPWMval = (PWMres * def_brightness) / brightness_steps;
    }
    swState = (float)_currentPWMval / (float)PWMres;

    // Dimming Uo
    for (int i = 0; i <= _currentPWMval; i = i + _PWMdimm_step)
    {
      analogWrite(_switchPin, i);
      delay(_PWMdimm_delay);
    }
    //  End Dimming
  }
  else
  {
    digitalWrite(_switchPin, RelayON_def);
    swState = 1.0;
  }
  if (TO != 0)
  {
    _timeout_mins = TO;
  }
  else
  {
    _timeout_mins = _stored_timeout;
  }
  _ONclock = millis();
}
void SensorSwitch::start()
{
  pinMode(_sensorPin, INPUT_PULLUP);
  pinMode(_switchPin, OUTPUT);

  if (useButton)
  {
    pinMode(_extPin, INPUT_PULLUP);
  }

  _sensorsState = digitalRead(_sensorPin);
  _last_sensorsState = digitalRead(_sensorPin);

  turnOff();
}
void SensorSwitch::checkButton()
{
  if (useButton)
  {
    if (digitalRead(_extPin) == ButtonPressed_def)
    {
      delay(50);
      if (digitalRead(_extPin) == ButtonPressed_def)
      {
        if (usePWM)
        {
          if (_currentPWMval - _PWMbutton_step >= 0)
          {
            _currentPWMval = _currentPWMval - _PWMbutton_step;
          }
          else
          {
            _currentPWMval = PWMres;
          }
          analogWrite(_switchPin, _currentPWMval);
          swState = (float)_currentPWMval / (float)PWMres;
          delay(200);
        }
        else
        {
          if ((int)round(swState) == 1)
          {
            turnOff();
          }
          else
          {
            turnOn();
          }
          delay(200);
        }
      }
    }
  }
}
void SensorSwitch::looper()
{
  checkSensor();
  checkButton();
  offBy_timeout();
}

void SensorSwitch::offBy_timeout()
{
  if (_timeout_mins * 1000ul * 60ul > 0 && _ONclock != 0)
  { // user setup TO ?
    if (millis() - _ONclock >= _timeout_mins * 1000ul * 60ul)
    { //TO ended
      turnOff();
      timeoutRem = 0;
    }
    timeoutRem = (int)((_ONclock + _timeout_mins * 1000ul * 60ul - millis()) / 1000);
  }
}
void SensorSwitch::checkSensor()
{
  _sensorsState = digitalRead(_sensorPin);
  if (_sensorsState != _last_sensorsState)
  { // enter on change only
    if (millis() - _lastDetect_clock > 100)
    { // ms of debounce
      if (_sensorsState == SensorDetection_def)
      {
        if (_bright_level < max_brightness)
        {
          _bright_level++;
        }
        turnOn();
      }
      else
      {
        if (_bright_level - 1 >= 0)
        {
          _bright_level--;
        }
        turnOff();
      }
      _lastDetect_clock = millis();
      _last_sensorsState = _sensorsState;
    }
  }
}

// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
UltraSonicSensor::UltraSonicSensor(byte trigPin, byte echoPin, int re_trigger_delay, int d_sensitivity)
{
  _trigPin = trigPin;
  _echoPin = echoPin;
  _re_trigger_delay = re_trigger_delay;
  dist_sensitivity = d_sensitivity;
}
void UltraSonicSensor::startGPIO()
{
  pinMode(_trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(_echoPin, INPUT);  // Sets the echoPin as an Input
}
int UltraSonicSensor::readSensor(int x, int del)
{
  long duration;
  int distance;
  int cum_distance = 0;
  int i = 0;

  while (i < x)
  {
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    duration = pulseIn(_echoPin, HIGH);
    distance = duration * 0.034 / 2;

    if (distance < _max_distance && distance > _min_distance)
    {
      i++;
      cum_distance += distance;
    }
    delay(del);
  }
  return (int)(cum_distance / i);
}
void UltraSonicSensor::marginReadings(int get_val, int &ret_read)
{
  if (get_val > _max_distance)
  {
    ret_read = _max_distance;
  }
  else if (get_val < _min_distance)
  {
    ret_read = _min_distance;
  }
  else
  {
    ret_read = get_val;
  }
}
bool UltraSonicSensor::check_detect()
{
  static int last_read = 0;
  static long detect_clock = 0;
  static bool in_detection = false;

  int S = 10;
  int del = 5;
  int curr_read = readSensor(S, del);
  if (in_detection && millis() - detect_clock > (_re_trigger_delay)*1000)
  {
    in_detection = false;
    return 0;
  }

  else if (!in_detection)
  {
    if (abs(last_read - curr_read) < dist_sensitivity)
    {
      // No detection
      last_read = curr_read;
      return 0;
    }
    else
    {
      int sec_read = readSensor(S, del); // secondary_read

      if (abs(last_read - sec_read) < dist_sensitivity)
      {
        // case a: this is a measuring error
        last_read = sec_read;
        Serial.println("Measuring error - no fuss");
        return 0;
      }
      else if (abs(curr_read - sec_read) < dist_sensitivity &&
               millis() - detect_clock > _re_trigger_delay * 1000 &&
               curr_read > min_dist_trig && curr_read < max_dist_trig)
      {
        // when 2 consqutive reading are close, and not as last - this is a detection
        Serial.print("Last: ");
        Serial.println(last_read);
        Serial.print("Current: ");
        Serial.println(curr_read);
        Serial.print("Secondary: ");
        Serial.println(sec_read);

        detect_clock = millis();
        last_read = curr_read;
        in_detection = true;
        detection_cb();
        // Serial.println("Detection!!");
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
}
void UltraSonicSensor::detection_cb()
{
  if (_use_detect_cb)
  {
    _detect_cb();
  }
}
void UltraSonicSensor::detect_cb(cb_func cb)
{
  // define by user to run external function.
  _use_detect_cb = true;
  _detect_cb = cb;
}
void UltraSonicSensor::end_detect_cb(cb_func cb)
{
  _use_end_detect_cb = true;
  _end_detect_cb = cb;
}