/* Written by Guy Dvir 30/08/2022
Its purpose is to control 3 state windows switch.
Features:
1) Auto off - to prevent holding relaty Up or down for infinite time
2) Lockdown - when signaled, shuts down window, and disables switch until lockdown is disabled.
3) Dual Sw - in case of need 2 switches to control the window.
4) Telemetry - psot change of state.
*/
#ifndef mywindowsw_h
#define mywindowsw_h

#include <Arduino.h>
#include <buttonPresses.h>
#include "defs.h"

#define off1 digitalWrite(_outpins[0], !RELAY_ON)
#define off2 digitalWrite(_outpins[1], !RELAY_ON)

// #define winSTOP \
//     off1        \
//         off2
//     if (!_virtWin) \
//     off1           \
//         off2

#define winUP      \
    if (!_virtWin) \
    digitalWrite(_outpins[0], RELAY_ON)
#define winDOWN    \
    if (!_virtWin) \
    digitalWrite(_outpins[1], RELAY_ON)

class WinSW
{
private:
    buttonPresses _windowSwitch;
    buttonPresses _windowSwitch_ext;

    bool _lockdownState = false;
    bool _uselockdown = false;
    bool _useTimeout = false;
    static uint8_t _next_id;
    int _timeout_clk = 0; // seconds to release relay
    unsigned long _timeoutcounter = 0;

public:
    uint8_t id = 0;
    bool newMSGflag = false;
    char name[40];
    bool _useExtSW = false;
    bool _virtWin = false;
    uint8_t _outpins[2];

    MSGstr MSG;

public:
    WinSW();
    void loop();
    void start();
    uint8_t get_id();
    void set_id(uint8_t i);
    void init_lockdown();
    void release_lockdown();
    void def_extSW(uint8_t upi = 255, uint8_t dpin = 255);
    void ext_SW(uint8_t state, uint8_t reason); /* External Callback */
    void def(uint8_t upin, uint8_t dpin, uint8_t outup_pin = 255, uint8_t outdown_pin = 255);
    void def_extras(bool useTimeout = true, bool useLockdown = true, int timeout_clk = 90);

    uint8_t get_winState();

private:
    void _winUP();
    void _allOff();
    void _readSW();
    void _winDOWN();
    void _timeout_looper();
    void _switch_cb(uint8_t state, uint8_t i);
};
#endif