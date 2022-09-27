#ifndef swmartSW_h
#define smartSW_h
#include <Arduino.h>
#include <Button2.h> /* Button Entities */

#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW
#define HWturnON(i) digitalWrite(i, OUTPUT_ON)
#define HWturnOFF(i) digitalWrite(i, !OUTPUT_ON)

struct MSGstr
{
    bool newMSG = false;
    uint8_t state;  /* Up/Down/ Off */
    uint8_t reason; /* What triggered the button */
};

class smartSwitch
{
#define _MQTT 1
#define MAX_NAME_LEN 35
#define UNDEF_PIN 255

public:
    char name[MAX_NAME_LEN];
    char *MQTTcmds[2] = {"on", "off"};
    MSGstr telemtryMSG;

public:
    smartSwitch();
    void set_id(uint8_t i);
    void set_extON(uint8_t opt = 0);
    void set_extOFF(uint8_t opt = 0);
    void set_name(char *Name);
    void set_output(uint8_t outpin = UNDEF_PIN);
    void set_input(uint8_t inpin = UNDEF_PIN, uint8_t t = 0);

    void turnON_cb(uint8_t type);
    void turnOFF_cb(uint8_t type);
    void loop();

    uint8_t get_id();
    uint8_t get_inpin();
    uint8_t get_outpin();
    void get_prefences();
    void get_telemetry(uint8_t state, uint8_t i);

    bool is_virtCMD();
    bool is_useButton();

private:
    uint8_t _id = 0;
    uint8_t _button_type = 255;
    uint8_t _outputPin = UNDEF_PIN;

    bool _virtCMD = false;
    bool _useButton = false;
    bool _guessState = false;

    static uint8_t _next_id;
    Button2 _inputButton; /* inputs only */

private:
    bool _isON();
    void _OnOffSW_Relay(uint8_t i, bool state, uint8_t type);
    void _toggleRelay(uint8_t i, uint8_t type);

    /* Button2 Handlers */
    void _toggle_handle(Button2 &b);
    void _OnOffSW_ON_handler(Button2 &b);
    void _OnOffSW_OFF_handler(Button2 &b);
};

#endif
