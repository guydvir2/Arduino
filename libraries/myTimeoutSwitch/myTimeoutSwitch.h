
#ifndef mytimeoutSW_h
#define mytimeoutSW_h
#include <Arduino.h>
#include <TimeLib.h>
#include <myJSON.h>

typedef void (*func_cb)(char msg1[50], byte i);

class timeOUTSwitch
{
public:
    const char *VER = "TOsw_v0.3";
    bool useInput = false;
    bool inTO = false;
    byte trigType = 0; /* 0: button/ momentary; 1:switch; 2: trigger; 3: step power ( example PWM )*/

    int def_TO_minutes = 1;
    unsigned int maxON_minutes = 100; // Max ON time
    unsigned int TO_duration_minutes = 0;
    unsigned long TO_start_millis = 0;
    unsigned long TO_endclk = 0;
    byte totPWMsteps = 3;
    byte pwm_pCount = 0;
    byte icount = 0;

    timeOUTSwitch(bool saveCLK = true);
    void def_funcs(func_cb startF, func_cb endF);
    void start_TO(int _TO, char *src);
    void finish_TO(char *src);
    void startIO(int _in_IO, bool _instate = HIGH);
    void looper();
    int remTime();
    void clearTO();

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
    char *_keyJSON = "end_clk_XXX";

    void _TOlooper();
    void _input_looper();
    void _updateEndClk(int TO_dur_minutes, unsigned long TO_start_clk = 0);
    void _chk_rem_after_boot();
};

#endif
