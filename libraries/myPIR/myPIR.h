#ifndef myPIR_h
#define myPIR_h

#include "Arduino.h"

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
    bool _isDetect = true;
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
    float ver = 2.1;

private:
    void update_timer_end();
    void detection_callback();
    void end_detection_callback();
    
    bool check_timer();
    bool checkSensor();

public:
    PIRsensor(int Pin, char *nick = "PIRsensor", int logic_length = 5, bool detect = def_detect);
    void start();
    void detect_cb(cb_func cb);
    void end_detect_cb(cb_func cb);
    void looper();
};

#endif
