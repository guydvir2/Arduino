#ifndef swmartSW_h
#define smartSW_h
#include <Arduino.h>
#include <Button2.h> /* Button Entities */
#include <Chrono.h>

#ifndef UNDEF_PIN
#define UNDEF_PIN 255
#endif

#ifndef MAX_TOPIC_SIZE
#define MAX_TOPIC_SIZE 40
#endif

#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW

struct SW_act_telem
{
    bool newMSG = false;
    uint8_t state = 255;  /* Up/Down/ Off */
    uint8_t reason = 255; /* What triggered the button */
    unsigned long clk_end = 0;
};
struct SW_props
{
    uint8_t id = 0;
    uint8_t type = 0;
    uint8_t inpin = UNDEF_PIN;
    uint8_t outpin = UNDEF_PIN;
    uint8_t indicpin = UNDEF_PIN;

    bool PWM = false;
    bool timeout = false;
    bool virtCMD = false;
    bool lockdown = false;
    char *name;
};

enum SWTypes : const uint8_t
{
    NO_INPUT,
    ON_OFF_SW,
    MOMENTARY_SW
};
enum InputTypes : const uint8_t
{
    BUTTON_INPUT,
    SW_TIMEOUT,
    EXT_0,
    EXT_1
};
enum SWstates : const uint8_t
{
    SW_OFF,
    SW_ON
};

/* "Virtcmd" is defined when output is not defined
   "useTimeout" is defined when set_timeout is set to t !=0
   "useButton" is defined when set_input is defined !=0;
*/
class smartSwitch
{
public:
    const char *ver = "smartSW_v0.4";
    char name[MAX_TOPIC_SIZE];
    SW_act_telem telemtryMSG;

public:
    smartSwitch();
    void set_id(uint8_t i);
    void set_timeout(int t = 0);
    void set_name(const char *Name);
    void set_lockSW();
    void set_unlockSW();
    void set_indiction(uint8_t pin = UNDEF_PIN, bool dir = 0);
    void set_useLockdown(bool t = true);
    void init_lockdown();
    void release_lockdown();
    void set_output(uint8_t outpin = UNDEF_PIN, uint8_t intense = 0);
    void set_input(uint8_t inpin = UNDEF_PIN, uint8_t t = 0);

    void turnON_cb(uint8_t type, unsigned int temp_TO = 0);
    void turnOFF_cb(uint8_t type);
    void clear_newMSG();
    bool loop();

    uint8_t get_SWstate();
    int get_remain_time();
    void get_SW_props(SW_props &props);

    bool useTimeout();
    bool is_virtCMD();
    bool is_useButton();
    Button2 _inputButton;

private:
    uint8_t _pwm_ints = 0;
    uint8_t _button_type = 255;
    uint8_t _outputPin = UNDEF_PIN;
    uint8_t _indicPin = UNDEF_PIN;

    bool _virtCMD = false;
    bool _useButton = false;
    bool _guessState = false;
    bool _use_timeout = false;
    bool _use_lockdown = false;
    bool _use_indic = false;
    bool _in_lockdown = false;
    bool _indic_on = false;
    bool _PWM_ison = false;
    bool _output_pwm = false;

    Chrono _timeout_clk;
    /* inputs only */
    unsigned long _timeout_duration = 1; // in seconds
    unsigned long _timeout_temp = 0;     // in seconds

private:
    bool _isON();
    void _HWon(uint8_t val = 0);
    void _HWoff();
    void _timeout_loop();
    void _stop_timeout();
    void _start_timeout();
    void _turn_indic_on();
    void _turn_indic_off();
    void _toggleRelay(uint8_t i, uint8_t type);
    void _OnOffSW_Relay(uint8_t i, bool state, uint8_t type);
    void _update_telemetry(uint8_t state, uint8_t type, unsigned long te = 0);

    /* Button2 Handlers */
    void _toggle_handle(Button2 &b);
    void _OnOffSW_ON_handler(Button2 &b);
    void _OnOffSW_OFF_handler(Button2 &b);
};

#endif
