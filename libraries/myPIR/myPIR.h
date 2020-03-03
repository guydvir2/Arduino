#ifndef myPIR_h
#define myPIR_h

#include "Arduino.h"

class PIRsensor
{
    typedef void (*cb_func)();

private:
    int _pin;
    int _length_logic_state = 5;
    bool _use_detfunc = false;
    bool _use_enddetfunc = false;
    bool _lastState = false;
    bool _isDetect = true;
    bool _timer_is_on = false;
    long _lastDetection_clock = 0;
    long _endTimer = 0;

    cb_func _run_func;
    cb_func _run_enddet_func;

public:
    int detCounts = 0;
    int ignore_det_interval = 20;   // seconds
    int timer_duration = 10;        // logic flag "1"
    int delay_first_detection = 15; // seconds
    char *sensNick = "sensor";
    bool use_timer = false;
    bool sens_state = false;
    bool logic_state = false; // a flag that will be on for some time, altghot physucal state has changed

    PIRsensor(int Pin, char *nick = "PIRsensor", int logic_length = 5);
    void start();
    void run_func(cb_func cb);
    void run_enddet_func(cb_func cb);
    bool checkSensor();
};

#endif
