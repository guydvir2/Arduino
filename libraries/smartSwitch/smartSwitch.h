#ifndef swmartSW_h
#define smartSW_h
#include <Arduino.h>
#include <Button2.h> /* Button Entities */
#include <Chrono.h>

#define UNDEF_PIN 255
#define MAX_NAME_LEN 35
#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW
#define HWturnON(i) digitalWrite(i, OUTPUT_ON)
#define HWturnOFF(i) digitalWrite(i, !OUTPUT_ON)

struct SW_MSGstr
{
    bool newMSG = false;
    uint8_t state;  /* Up/Down/ Off */
    uint8_t reason; /* What triggered the button */
    unsigned long clk_start = 0;
    unsigned long clk_end = 0;
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
class smartSwitch
{
public:
    char ver[15] = "smartSW_v0.1";
    char name[MAX_NAME_LEN];
    SW_MSGstr telemtryMSG;

public:
    smartSwitch();
    void set_id(uint8_t i);
    void set_name(char *Name);
    void set_timeout(int t);
    void set_output(uint8_t outpin = UNDEF_PIN);
    void set_input(uint8_t inpin = UNDEF_PIN, uint8_t t = 0);

    void turnON_cb(uint8_t type, unsigned int temp_TO = 0);
    void turnOFF_cb(uint8_t type);
    void clear_newMSG();
    bool loop();

    uint8_t get_id();
    uint8_t get_inpin();
    uint8_t get_outpin();
    uint8_t get_SWtype();
    uint8_t get_SWstate();
    int get_remain_time();
    void get_prefences();
    void get_telemetry(uint8_t state, uint8_t reason);

    bool useTimeout();
    bool is_virtCMD();
    bool is_useButton();
    Button2 _inputButton;  

private:
    uint8_t _button_type = 255;
    uint8_t _outputPin = UNDEF_PIN;

    bool _virtCMD = false;
    bool _useButton = false;
    bool _guessState = false;
    bool _use_timeout = false;

    Chrono _timeout_clk;
                  /* inputs only */
    unsigned long _timeout_duration = 1; // in seconds
    unsigned long _timeout_temp = 0; // in seconds

private:
    bool _isON();
    void _timeout_loop();
    void _update_telemetry(uint8_t state, uint8_t type);
    void _OnOffSW_Relay(uint8_t i, bool state, uint8_t type);
    void _toggleRelay(uint8_t i, uint8_t type);
    void _stop_timeout();
    void _start_timeout();

    /* Button2 Handlers */
    void _toggle_handle(Button2 &b);
    void _OnOffSW_ON_handler(Button2 &b);
    void _OnOffSW_OFF_handler(Button2 &b);
};

#endif
