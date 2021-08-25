
#ifndef mytimeoutSW_h
#define mytimeoutSW_h
#include <Arduino.h>
#include <TimeLib.h>
#include <myJSON.h>

typedef void (*func_cb)(uint8_t src, uint8_t i);

class timeOUTSwitch
{
public:
    const char *VER = "TOsw_v0.8";
    bool useInput = false;
    bool inTO = false;
    uint8_t trigType = 0; /* 
                          0: momentary button + in PWM pulse counter / PWM
                          1:switch; 
                          2:trigger/ sensor; 
                          */

    int def_TO_minutes = 1;           // time when input pressed
    unsigned int maxON_minutes = 100; // Max ON-time
    unsigned int TO_duration = 0;
    unsigned long TO_start_millis = 0;
    unsigned long TO_endclk = 0;
    uint8_t max_pCount = 3;
    uint8_t pCounter = 0;
    uint8_t icount = 0;

    timeOUTSwitch(bool saveCLK = true);
    void def_funcs(func_cb startF, func_cb endF);
    void start_TO(int _TO, uint8_t src, bool minutes = true);
    void finish_TO(uint8_t src);
    void startIO(int _in_IO, bool _instate = HIGH, bool _reverseInput = false);
    void looper();
    void clearTO();
    int remTime();
    time_t onClk();
    uint8_t getCount();

    myJSON CLKstore;

private:
    uint8_t _IN_io = 1;
    bool _inputstatOn = HIGH;
    bool _lastinput = !_inputstatOn;
    bool _useSavedCLK = false;
    unsigned long _lastPress = 0;

    func_cb _startf;
    func_cb _endf;
    char _keyEnd[12];
    char _keyStart[12];
    char _keyCounter[12];

    void _TOlooper();
    void _input_looper();
    void _updateEndClk(int TO_dur_minutes, unsigned long TO_start_clk = 0);
    void _updateStartClk(long TO_start_clk);
    void _chk_rem_after_boot();
    time_t _now();
};

#endif
