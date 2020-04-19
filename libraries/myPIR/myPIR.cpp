#include "Arduino.h"
#include "myPIR.h"

PIRsensor::PIRsensor(int Pin, char *nick, int logic_length, bool detect)
{
  _pin = Pin;
  _length_logic_state = logic_length;
  _isDetect = detect;
  sensNick = nick;
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
    Serial.print("~~detect ");
    Serial.println(sensNick);
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
    Serial.print("~~end_detect ");
    Serial.println(sensNick);
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
  _ONclock = 0;
  swState = 0.0;
}
void SensorSwitch::turnOn(int TO)
{
  if (usePWM)
  {
    if (_currentPWMval == 0)
    {
      _currentPWMval = 0.6 * _maxPWM;
    }
    for (int i = 0; i <= _currentPWMval; i = i + _PWMdimm_step)
    {
      analogWrite(_switchPin, i);
      delay(_PWMdimm_delay);
    }
  }
  else
  {
    digitalWrite(_switchPin, RelayON_def);
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
  swState = 1.0;
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
            _currentPWMval = _maxPWM;
          }
          analogWrite(_switchPin, _currentPWMval);
          swState = (float)_currentPWMval / (float)_maxPWM;
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
      if (_sensorsState == !SensorDetection_def)
      {
        turnOn();
      }
      else
      {
        turnOff();
      }
      _lastDetect_clock = millis();
      _last_sensorsState = _sensorsState;
    }
  }
}

UltraSonicSensor::UltraSonicSensor(byte trigPin, byte echoPin, int re_trigger_delay, byte dist_sensitivity)
{
  _trigPin = trigPin;
  _echoPin = echoPin;
  _re_trigger_delay = re_trigger_delay;
  _dist_sensitivity = dist_sensitivity;
}

void UltraSonicSensor::startGPIO()
{
  pinMode(_trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(_echoPin, INPUT);  // Sets the echoPin as an Input
}
void UltraSonicSensor::detection_cb()
{
  Serial.println("Detect");
}

float UltraSonicSensor::arrayofmeasurements(int arr_size, char *name)
{
  int sum = 0;

  for (int a = 0; a < arr_size; a++)
  {
    int x = readSensor();
    if (x > _max_distance)
    {
      x = _max_distance;
    }
    else if (x < _min_distance)
    {
      x = _min_distance;
    }
    // Serial.println(x);
    sum += x;
    delay(20);
  }
  return (float)sum / (float)arr_size;
}

int UltraSonicSensor::readSensor()
{
  long duration;
  int distance;

  digitalWrite(_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trigPin, LOW);

  duration = pulseIn(_echoPin, HIGH);
  distance = duration * 0.034 / 2;

  return distance;
}

bool UltraSonicSensor::check_detect()
{
  static long last_detect = 0;
  const byte arr_size = 30;

  float mean_a = arrayofmeasurements(arr_size);
  delay(100);
  float mean_b = arrayofmeasurements(arr_size);

  if (abs(mean_b - mean_a) > _dist_sensitivity && millis() - last_detect >= _re_trigger_delay * 1000) // && max(mean_a,mean_b)<max_det_distance
  {
    detection_cb();
    last_detect = millis();
    char t[50];
    sprintf(t, "Detection at A=%.1f B=%.1f, delta=%.1f", mean_a, mean_b, abs((mean_a - mean_b) / 2.0));
    Serial.println(t);
    _detect_cb();
    return 1;
  }
  else
  {
    return 0;
  }
}
void UltraSonicSensor::detect_cb(cb_func cb)
{
  _use_detect_cb = true;
  _detect_cb = cb;
}
void UltraSonicSensor::end_detect_cb(cb_func cb)
{
  _use_end_detect_cb = true;
  _end_detect_cb = cb;
}