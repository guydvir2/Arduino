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
  Serial.println("update");
}

bool PIRsensor::check_timer()
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
        return 0;
      }
    }
    else
    {
      // Timer remains
      timeLeft = (int)((_endTimer - millis()) / 1000);
      return 1;
    }
  }
  else
  {
    return 0;
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
      if (trigger_once == false){
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
  // _lastDetection_clock = 0;
  if (use_serial)
  {
    Serial.print("~~end_detect ");
    Serial.println(sensNick);
  }
}

bool PIRsensor::checkSensor()
{
  bool ignore_det = millis() > _lastDetection_clock + (long)((ignore_det_interval + _length_logic_state) * 1000); // timeout - minimal time between detections
  // bool first_det = millis() > (long)delay_first_detection * 1000;                                                 // timeout - detection after boot

  logic_state = millis() <= _lastDetection_clock + (long)(_length_logic_state * 1000); // logic flag for sensor to be indetection mode
  sens_state = digitalRead(_pin);                                                      // physical sensor read

  if (logic_state == false)
  {
    if (sens_state == _isDetect && _lastState == false)
    {
      if (ignore_det || _lastDetection_clock == 0)

      {
        detection_callback();
        return 1;
      }
      else
      {
        return 0;
      }
    }
    else if (sens_state != _isDetect && _lastState)
    {
      end_detection_callback();
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

void PIRsensor::looper()
{
  if (stop_sensor == false)
  {
    check_timer();
    checkSensor();
  }
  // else{
  //   yield();
  // }
}
