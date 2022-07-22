#ifndef TOB_h
#define TOB_h

#include <Arduino.h>
#include <Button2.h>
#include <Chrono.h>

class timeoutButton
{
private:
    typedef void (*cb_func)(uint8_t resaon, uint8_t i);
#define conv2Minute(t) t * 60

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
        BUTTON
    };

    uint8_t _pin = 255;
    uint8_t _trigType = 0;
    uint8_t _pressCounter = 0;
    unsigned long _lastPress = 0;

private:
    Chrono chrono;
    Button2 button;

    cb_func _extOn_cb = nullptr;
    cb_func _extOff_cb = nullptr;
    cb_func _extMultipress_cb = nullptr;

public:
    bool useInput = false;
    int timeout = 0;
    int maxTimeout = 500;
    int defaultTimeout = 1;
    int time2Repress = 1000; // millis
    uint8_t _id = 0;

protected:
    const char *ver = "timeouter_v0.3";

private:
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

    void OFF_cb(uint8_t reason);         // const char *trigger);
    void ON_cb(int _TO, uint8_t reason); //, const char *trigger = nullptr, uint8_t _PWMstep = 1);
    void ExtON_cb(cb_func func);
    void ExtOFF_cb(cb_func func);
    void ExtMultiPress_cb(cb_func func);
};
#endif
