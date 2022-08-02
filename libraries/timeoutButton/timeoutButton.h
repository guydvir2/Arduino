#ifndef TOB_h
#define TOB_h

#include <Arduino.h>
#include <Button2.h>
#include <Chrono.h>
#include <TurnOnLights.h>
#include <FS.h>

#if defined(ESP32)
#include "LITTLEFS.h"
#define LITFS LITTLEFS
#elif defined(ESP8266)
#include <LittleFS.h>
#define LITFS LittleFS
#endif

struct oper_string
{
    bool state;     /* On or Off */
    uint8_t step;   /* Step, in case of PWM */
    uint8_t reason; /* What triggered the button */
    time_t ontime;  /* Start Clk */
    time_t offtime; /* Off Clk */
};
enum SWITCH_TYPES : const uint8_t
{
    MOMENTARY,
    ON_OFF,
    TRIGGER_SERNSOR,
    MULTI_PRESS
};
enum REASONS : const uint8_t
{
    TIMEOUT,
    BUTTON,
    MQTT,
    PWRON,
    REBOOT
};

class timeoutButton
{
#define conv2Minute(t) t * 60

private:
    const char *INPUT_ORG[5] = {"Timeout", "Button", "MQTT", "PowerON", "Resume Reboot"};

    unsigned long _lastPress = 0;
    bool _useInput = false;
    char _operfile[15];

private:
    Chrono chrono;
    Button2 button;

public:
    int timeout = 0;
    int maxTimeout = 500;
    int defaultTimeout = 1;
    int time2Repress = 1000; // millis
    oper_string OPERstring = {false, 0, 0, 0, 0};
    bool newMSG = false;
    uint8_t pressCounter = 0;
    uint8_t trigTYPE = 0;
    uint8_t Id = 0;
    uint8_t Inpin = 255;

protected:
    const char *ver = "timeouter_v0.3";

private:
    void _commonBegin(uint8_t id);
    void _init_button();
    void _Button_looper();
    void _ON_OFF_on_handle(Button2 &b);
    void _Momentary_handle(Button2 &b);
    void _ON_OFF_off_handle(Button2 &b);
    void _TrigSensor_handler(Button2 &b);
    void _MultiPress_handler(Button2 &b);

    void _stopWatch();
    void _loopWatch();
    void _startWatch();
    void _init_chrono();

public:
    timeoutButton();
    void begin(uint8_t id); /* Not using button */
    void begin(uint8_t pin, uint8_t trigType, uint8_t id = 0);
    void addWatch(int _add, uint8_t reason);

    void loop();
    bool getState();
    unsigned int remainWatch();

    void OFF_cb(uint8_t reason);
    void ON_cb(int _TO, uint8_t reason);

    void save_OperStr(oper_string &str);
    void read_OperStr(oper_string &str);
    void print_OPERstring(oper_string &str);
};

class LightButton : private TurnOnLights
{
private:
    uint8_t _buttonID = 0;

public:
    bool OnatBoot = false;
    bool outputPWM = false;

    bool inputPressed = LOW; // NEED TO DEFINED IN BUTTON CLASS //
    bool &newMSG = Button.newMSG;
    uint8_t &trigType = Button.trigTYPE;
    uint8_t &inputPin = Button.Inpin;
    int &def_TO_minutes = Button.defaultTimeout;
    int &maxON_minutes = Button.maxTimeout;

    bool &output_ON = _isON;
    bool &dimmablePWM = _useDim;
    uint8_t &outputPin = Pin;
    uint8_t &indicPin = auxPin;
    uint8_t &defPWM = defStep;
    uint8_t &max_pCount = maxSteps;
    uint8_t &limit_PWM = limitPWM;
    int &PWM_res = PWMres;

    oper_string *OPstr = &Button.OPERstring;

private:
    timeoutButton Button;
    void _newActivity_handler();
    void _init_button();
    void _init_light();
    void _init_onAtBoot();
    void _turnONlights();
    void _turnOFFlights();

public:
    LightButton();
    void sendMSG(oper_string &str);

    // ~~~~~~~~ Belongs to Button Class ~~~~~
    void loop();
    bool getState();
    void begin(uint8_t id);
    unsigned int remainWatch();
    void OFF_cb(uint8_t reason);
    void ON_cb(int _TO, uint8_t reason);
};
#endif
