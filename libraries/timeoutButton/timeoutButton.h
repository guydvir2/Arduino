#ifndef TOB_h
#define TOB_h

#include <Arduino.h>
#include <Button2.h>
#include <Chrono.h>

class timeoutButton
{
private:
    typedef void (*cb_func)();
    enum SWITCH_TYPES : const uint8_t
    {
        MOMENTARY,
        ON_OFF,
        TRIGGER_SERNSOR,
        BTN_TO_PWM,
        MULTI_PRESS
    };

    const char *SRCS[4] = {"Button", "Timeout", "MQTT", "PowerOn"};

    uint8_t _id = 0;
    uint8_t _pin = 255;
    uint8_t _trigType = 0;
    uint8_t _aux_pin = 255;

private:
    Button2 button;
    Chrono chrono;
    cb_func _extOff_cb;
    cb_func _extOn_cb;

public:
    bool useInput = false;
    int timeout = 0;
    int maxTimeout = 500;
    int defaultTimeout = 10;

protected:
    const char *ver = "timeouter_v0.1";

private:
    void _init_button();
    void _ON_OFF_on_handle(Button2 &b);
    void _ON_OFF_off_handle(Button2 &b);
    void _Momentary_handle(Button2 &b);
    void _TrigSensor_handler(Button2 &b);
    void _Button_looper();

    void _init_chrono();
    void _stopWatch();
    void _startWatch();
    void _loopWatch();

public:
    timeoutButton();
    void begin(); /* Not using button */
    void begin(uint8_t pin, uint8_t trigType = 0, uint8_t id = 0);
    void addWatch(int _add, const char *trigger = nullptr);
    unsigned int remainWatch();

    void OFF_cb();           // const char *trigger);
    void ON_cb(uint8_t _TO); //, const char *trigger = nullptr, uint8_t _PWMstep = 1);
    void ExtON_cb(cb_func func);
    void ExtOFF_cb(cb_func func);
    void loop();

    bool getState();
};
#endif
