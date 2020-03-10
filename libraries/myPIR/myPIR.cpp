#include "Arduino.h"
#include "myPIR.h"

PIRsensor::PIRsensor(int Pin, char *nick, int logic_length)
{
  _pin = Pin;
  _length_logic_state = logic_length;
  sensNick = nick;
}
void PIRsensor::start()
{
  pinMode(_pin, INPUT);
  digitalWrite(_pin, !_isDetect);
}
void PIRsensor::run_func(cb_func cb)
{
  _run_func = cb;
  _use_detfunc = true;
}
void PIRsensor::run_enddet_func(cb_func cb)
{
  _run_enddet_func = cb;
  _use_enddetfunc = true;
}

bool PIRsensor::checkSensor()
{
  if (stop_sensor==false)
  {
    bool ignore_det = millis() > _lastDetection_clock + (long)((ignore_det_interval + _length_logic_state) * 1000); // timeout - minimal time between detections
    bool first_det = millis() > (long)delay_first_detection * 1000;                                                 // timeout - detection after boot
    if (use_timer && _timer_is_on && millis() > _endTimer)
    {
      _timer_is_on = false;
    }
    logic_state = millis() <= _lastDetection_clock + (long)(_length_logic_state * 1000); // logic flag for sensor to be indetection mode
    sens_state = digitalRead(_pin);                                                      // physical sensor read
    if (sens_state == _isDetect && _lastState == false && logic_state == false && _timer_is_on == false)
    {
      if (ignore_det || first_det)
      {
        Serial.print("~~detect_");
        Serial.println(sensNick);
        detCounts++;
        _lastState = true;
        _lastDetection_clock = millis();
        if (_use_detfunc)
        {
          _run_func();
        }
        if (use_timer)
        {
          _endTimer = millis() + (long)timer_duration * 1000;
          _timer_is_on = true;
        }
        return 1;
      }
    }
    else if (sens_state != _isDetect && _lastState && logic_state == false)
    {
      _lastState = false;
      _lastDetection_clock = 0;
      Serial.print("~~END_");
      Serial.println(sensNick);
      return 0;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}
