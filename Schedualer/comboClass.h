#include <Chrono.h>
#include <Button2.h>

class timeouter
{

private:
    enum TRIG_SRC : const uint8_t
    {
        BTN,
        TIMEOUT,
        MQTT_CMD,
        PWR_ON,
    };
    enum SWITCH_TYPES : const uint8_t
    {
        MOMENTARY,
        ON_OFF,
        TRIGGER_SERNSOR,
        BTN_TO_PWM,
        MULTI_PRESS
    };

    const char *SRCS[4] = {"Button", "Timeout", "MQTT", "PowerOn"};

    int maxTimeout;
    int defaultTimeout;
    int lastPWMvalue;
    int timeout;

    uint8_t _id = 0;
    uint8_t _aux_pin = 255;
    uint8_t _pin = 255;
    uint8_t _trigType = 0;

private:
    Button2 button;
    Chrono chrono;

public:
    bool useInput = false;

private:
    void _init_button()
    {
        if (useInput)
        {
            button.begin(_pin);
            button.setID(_id);

            if (_trigType == MOMENTARY)
            {
                // button.setPressedHandler(_Momentary_handle);
            }
            else if (_trigType == ON_OFF)
            {
                // button.setPressedHandler(_ON_OFF_on_handle);
                // button.setReleasedHandler(_ON_OFF_off_handle);
            }
            else if (_trigType == TRIGGER_SERNSOR)
            {
                // button.setPressedHandler(_TrigSensor_handler);
            }
            else if (_trigType == BTN_TO_PWM)
            {
            }
            else if (_trigType == MULTI_PRESS)
            {
            }
        }
    }
    void _ON_OFF_on_handle(Button2 &b)
    {
        ONcmd(timeout, SRCS[BTN]);
    }
    void _ON_OFF_off_handle(Button2 &b)
    {
        OFFcmd(SRCS[BTN]);
    }
    void _Momentary_handle(Button2 &b)
    {
        if (chrono.isRunning()) /* Turn OFF if ON */
        {
            OFFcmd(SRCS[BTN]);
        }
        else
        {
            ONcmd(timeout, SRCS[BTN]);
        }
    }
    void _TrigSensor_handler(Button2 &b)
    {
        const uint8_t update_timeout = 30; // must have passed this amount of seconds to updates timeout
        unsigned int _remaintime = remainWatch();

        if (_remaintime != 0 && timeout - _remaintime > update_timeout)
        {
            startWatch(); /* Restart timeout after 30 sec */
        }
    }
    void _Button_looper()
    {
        if (useInput)
        {
            button.loop();
        }
    }

    void _init_chrono()
    {
        _stopWatch();
    }
    void _stopWatch()
    {
        chrono.restart();
        chrono.stop();
        timeout = 0;
    }
    void _startWatch()
    {
        chrono.restart();
    }
    unsigned int remainWatch()
    {
        if (chrono.isRunning())
        {
            return chrono.elapsed();
        }
        else
        {
            return 0;
        }
    }

    void _loopWatch()
    {
        if (chrono.isRunning() && chrono.hasPassed(timeout * 60))
        {
            OFFcmd(SRCS[TIMEOUT]);
        }
    }

public:
    timeouter() : chrono(Chrono::SECONDS)
    {
    }
    void begin(uint8_t pin, uint8_t trigType, uint8_t id)
    {
        _pin = pin;
        _trigType = trigType;
        _id = id;

        _init_button();
        _init_chrono();
    }
    void addWatch(int _add, const char *trigger = nullptr)
    {
        timeout += _add;

        if (!chrono.isRunning()) /* Case not ON */
        {
            ONcmd(timeouts, trigger);
        }
    }

    void ONcmd(uint8_t _TO, const char *trigger, uint8_t _PWMstep)
    {
        if (!chrono.isRunning()) /* Enter when off or at PWM different PWM value */
        {
            _TO == 0 ? timeout = defaultTimeout : timeouts = _TO;
            _startWatch;
            // lightVector[i]->turnON(_PWMstep);
            // notifyON(i, trigger);
        }
    }
    void OFFcmd(const char *trigger)
    {
        if (chrono.isRunning())
        {
            int x = chrono.elapsed();
            // notifyOFF(i, x, trigger);
            _stopWatch();
            // lightVector[i]->turnOFF();
        }
    }
    void loop()
    {
        _loopWatch();
        _Button_looper();
    }

protected:
    const char *ver = "timeouter_v0.1";
};