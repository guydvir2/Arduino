
#ifndef mytimeoutSW_h
#define mytimeoutSW_h
#include <Arduino.h>
#include <TimeLib.h>
#include <myJSON.h>

typedef void (*func_cb)(char msg1[50], byte i);

class timeOUTSwitch
{
public:
    const char *VER = "TOsw_v0.5";
    bool useInput = false;
    bool inTO = false;
    bool outputPWM = false;
    byte trigType = 0; /* 
                          0: momentary button; 
                          1:switch; 
                          2:trigger/ sensor; 
                          */

    int def_TO_minutes = 1; // time when input pressed
    unsigned int maxON_minutes = 100; // Max ON-time
    unsigned int TO_duration = 0;
    unsigned long TO_start_millis = 0;
    unsigned long TO_endclk = 0;
    byte totPWMsteps = 3;
    byte pwm_pCount = 0;
    byte icount = 0;

    timeOUTSwitch(bool saveCLK = true);
    void def_funcs(func_cb startF, func_cb endF);
    void start_TO(int _TO, char *src, bool minutes = true);
    void finish_TO(char *src);
    void startIO(int _in_IO, bool _instate = HIGH);
    void looper();
    void clearTO();
    int remTime();
    time_t onClk();

    myJSON CLKstore;

private:
    byte _IN_io = 1;
    // static byte _counter;
    bool _inputstatOn = HIGH;
    bool _lastinput = !_inputstatOn;
    bool _useSavedCLK = false;
    unsigned long _lastPress = 0;

    func_cb _startf;
    func_cb _endf;
    char _keyEnd[10];
    char _keyStart[10];

    void _TOlooper();
    void _input_looper();
    void _updateEndClk(int TO_dur_minutes, unsigned long TO_start_clk = 0);
    void _updateStartClk(long TO_start_clk);
    void _chk_rem_after_boot();
};

#endif
