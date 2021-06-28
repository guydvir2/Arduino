
#ifndef mytimeoutSW_h
#define mytimeoutSW_h
#include <Arduino.h>
#include <TimeLib.h>
#include <myJSON.h>

typedef void (*func_cb)(char msg1[50]);

class timeOUTSwitch
{
public:
    const char *VER = "TOsw_v0.1";
    bool useInput = false;
    bool isMomentary = false;
    bool inTO = false;
    byte trigType = 0; /* 0: button/ momentary; 1:switch; 2: trigger*/

    const int def_TO_minutes = 10;
    unsigned int maxON_minutes = 100; // Max ON time
    unsigned int TO_duration_sec = 0;
    unsigned long TO_start_millis = 0;
    unsigned long TO_endclk = 0;

    timeOUTSwitch(bool saveCLK = true);
    void def_funcs(func_cb startF, func_cb endF);
    void start_TO(int _TO, char *src);
    void finish_TO(char *src);
    void restart_TO(int _TO = 0);
    void startIO(int _in_IO, bool _instate = HIGH);
    void looper();
    int remTime();
    void clearTO();

    myJSON CLKstore;

private:
    int _IN_io = 1;
    bool _inputstatOn = HIGH;
    bool _lastinput = !_inputstatOn;
    bool _useSavedCLK = false;
    func_cb _startf;
    func_cb _endf;
    const char *_keyJSON = "end_clk";

    void _TOlooper();
    void _input_looper();
    void _updateEndClk(int TO_dur_minutes, unsigned long TO_start_clk = 0);
    void _chk_rem_after_boot();
};

#endif
