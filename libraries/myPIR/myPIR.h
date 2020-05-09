#ifndef myPIR_h
#define myPIR_h

#include "Arduino.h"
#include <HCSR04.h>


class PIRsensor
{
    typedef void (*cb_func)();
#define def_detect HIGH

private:
    int _pin;
    int _length_logic_state = 5;
    bool _use_detfunc = false;
    bool _use_enddetfunc = false;
    bool _lastState = false;
    bool _isDetect = HIGH;
    bool _timer_is_on = false;
    bool _first_det = false;
    long _lastDetection_clock = 0;
    long _endTimer = 0;

    cb_func _detect_cb;
    cb_func _end_detect_cb;

public:
    int detCounts = 0;
    int timeLeft = 0;

    int ignore_det_interval = 20;   // seconds
    int timer_duration = 10;        // logic flag "1"
    int delay_first_detection = 15; // seconds

    char *sensNick = "sensorNAMES";

    bool use_serial = false;  // select to use serial port
    bool use_timer = false;   // select if detection will rise a flag for predefined time
    bool sens_state = false;  // select detection is HIGH or LOW
    bool logic_state = false; // a flag that will be on for some time, altghot physucal state has changed
    bool stop_sensor = false; // during code run, select to disable sensor activity
    bool trigger_once = true; // when using timer, how to react to re-detect ? add time ?
    float ver = 2.2;

private:
    void update_timer_end();
    void detection_callback();
    void end_detection_callback();
    void check_timer();
    void checkSensor();

public:
    PIRsensor(int Pin, char *nick = "PIRsensor", int logic_length = 5, bool detect = def_detect);
    void start();
    void detect_cb(cb_func cb);
    void end_detect_cb(cb_func cb);
    void looper();
};

class SensorSwitch
{
private:
    byte _switchPin, _extPin, _sensorPin;
    int _timeout_mins, _stored_timeout;

    // PWM settings
    byte _maxPWM = 240; // Arduino 256, ESP 1024
    byte _PWMbutton_step = 60;
    byte _PWMdimm_step = 20;
    byte _PWMdimm_delay = 30;
    byte _currentPWMval = _maxPWM;
    long _PWMdiff = 0.1;

    bool _sensorsState, _last_sensorsState;
    long unsigned _ONclock = 0;
    long unsigned _lastDetect_clock = 0;

public:
    float swState = 0.0;
    int timeoutRem = 0;
    bool useButton = false;
    bool usePWM = false;
    bool RelayON_def = true;
    bool ButtonPressed_def = LOW;
    bool SensorDetection_def = LOW;

public:
    SensorSwitch(byte sensorPin, byte switchPin, int timeout_mins = 10, byte extPin = 0);
    void turnOff();
    void turnOn(int TO = 0);
    void start();
    void checkButton();
    void looper();

private:
    void offBy_timeout();
    void checkSensor();
};

class UltraSonicSensor
{
    typedef void (*cb_func)();

private:
    byte _trigPin;
    byte _echoPin;
    int _re_trigger_delay;
    const int _max_distance = 350;
    const int _min_distance = 1;

    cb_func _detect_cb;
    cb_func _end_detect_cb;
    bool _use_detect_cb = false;
    bool _use_end_detect_cb = false;

private:
    int readSensor(int x=1, int del=0);
    void marginReadings(int get_val, int &ret_read);
    void detection_cb();

public:
    int dist_sensitivity;
    int min_dist_trig = 20;
    int max_dist_trig = 150;

public:
    UltraSonicSensor(byte trigPin, byte echoPin, int re_trigger_delay = 30, int d_sensitivity = 5);
    void startGPIO();
    bool check_detect();
    void detect_cb(cb_func cb);
    void end_detect_cb(cb_func cb);
};
#endif
