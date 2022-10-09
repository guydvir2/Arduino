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
#include "defs.h"

#define winUP     \
    if (!virtCMD) \
    digitalWrite(outpins[0], RELAY_ON)

#define winDOWN   \
    if (!virtCMD) \
    digitalWrite(outpins[1], RELAY_ON)

class RockerSW
{
#define UNDEF_INPUT 255
#define PRESSED LOW
#define DEBOUNCE_MS 50

    enum state : const uint8_t
    {
        STATE_OFF,
        STATE_1,
        STATE_2,
        STATE_NOCHG,
        STATE_ERR
    };

private:
    unsigned long _down_ms[2] = {0, 0};
    unsigned long _down_rel_ms[2] = {0, 0};
    bool _is_triggered[2] = {false, false};

    uint8_t _state[2] = {false, false};
    uint8_t _last_state[2] = {false, false};
    uint8_t _pins[2] = {UNDEF_INPUT, UNDEF_INPUT};

public:
    RockerSW();
    uint8_t get_raw();
    uint8_t get_SWstate();
    uint8_t get_pins(uint8_t i);
    void set_input(uint8_t upPin, uint8_t downPin, uint8_t active_dir = INPUT_PULLUP);

private:
    uint8_t _readPin(uint8_t i);

friend class WinSW;
};

class WinSW
{
#define RELAY_ON HIGH
#define MAX_NAME_LEN 40
#define UNDEF_INPUT 255

private:
    RockerSW _mainSW;
    RockerSW _extSW;

    uint8_t _id = 0;
    static uint8_t _next_id; /* Instance counter */

    bool _lockdownState = false;
    bool _uselockdown = false;
    int _timeout_clk = 0; // seconds to release relay
    unsigned long _timeoutcounter = 0;

public:
    bool virtCMD = false;
    bool useExtSW = false;
    bool newMSGflag = false;

    char ver[14] = "WinSW_v0.4";
    char name[MAX_NAME_LEN] = {""};
    uint8_t outpins[2];

    MSGstr MSG;

public:
    WinSW();
    bool loop();
    void init_lockdown();
    void release_lockdown();

    void set_id(uint8_t i);
    void set_name(const char *_name);
    void set_input(uint8_t upin, uint8_t dpin);
    void set_WINstate(uint8_t state, uint8_t reason); /* External Callback */
    void set_ext_input(uint8_t upi = UNDEF_INPUT, uint8_t dpin = UNDEF_INPUT);
    void set_output(uint8_t outup_pin = UNDEF_INPUT, uint8_t outdown_pin = UNDEF_INPUT);
    void set_extras(bool useLockdown = true, int timeout_clk = 90);

    uint8_t get_id();
    uint8_t get_winState();

    void clear_newMSG();
    void print_preferences();

private:
    void _winUP();
    void _allOff();
    void _readSW();
    void _winDOWN();
    void _timeout_looper();
    void _switch_cb(uint8_t state, uint8_t i);
};
#endif