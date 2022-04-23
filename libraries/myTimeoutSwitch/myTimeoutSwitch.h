#ifndef mytimeoutSW_h
#define mytimeoutSW_h
#include <Arduino.h>
#include <time.h>
#include <myJSON.h>

typedef void (*func_cb)(uint8_t src, uint8_t i);

class timeOUTSwitch
{
public:
    const char *Ver = "TOsw_v1.0a";
    bool useInput = false;
    bool inTO = false;
    uint8_t icount = 0;
    uint8_t pCounter = 0;
    uint8_t trigType = 0; /* 0: momentary button; 1: O-OFF switch; 2: trigger/ sensor; 3: PWM pulse counter / PWM ; 4: Add time each press + long press to end */
    uint8_t max_pCount = 3;

    unsigned int TO_duration = 0;
    unsigned int def_TO_minutes = 5;  // time when input pressed
    unsigned int maxON_minutes = 100; // Max ON-time
    unsigned long TO_start_millis = 0;

public:
    timeOUTSwitch(bool saveCLK = true);
    void def_funcs(func_cb startF, func_cb endF);
    void startIO(int _in_IO, bool _instate = HIGH);

    void looper();
    void clearTO();
    void finish_TO(uint8_t src);
    void add_TO(int _TO, uint8_t src, bool minutes = true);
    void start_TO(int _TO, uint8_t src, bool minutes = true);

    int remTime();
    time_t onClk();
    uint8_t getCount(bool forceF = false); // forceF to force read Flash stored value (case of reboot)
    void updatePcount(uint8_t val);
    void updateEndClk(int TO_dur_minutes, unsigned long TO_start_clk = 0);

    myJSON CLKstore;

private:
    uint8_t _IN_io = 1;
    bool _inputstatOn = HIGH;
    bool _useSavedCLK = false;
    bool _lastinput = !_inputstatOn;
    unsigned long _lastPress = 0;

    func_cb _endf;
    func_cb _startf;

    char _keyEnd[12];
    char _keyStart[12];
    char _keyCounter[12];

    time_t _now();
    void _TOlooper();
    void _input_looper();
    void _updateStartClk(unsigned long TO_start_clk);
    void _chk_rem_after_boot();
};

#endif
