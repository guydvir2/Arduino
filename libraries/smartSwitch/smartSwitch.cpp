#include <Arduino.h>
#include "smartSwitch.h"

smartSwitch::smartSwitch() : _inputButton(),
                             _timeout_clk(Chrono::MILLIS)
{
}

void smartSwitch::set_id(uint8_t i)
{
    _inputButton.setID(i);
}
void smartSwitch::set_timeout(int t)
{
    if (t > 0)
    {
        _use_timeout = true;
        _timeout_duration = t * 1000;
        _stop_timeout();
    }
}
void smartSwitch::set_name(const char *Name)
{
    strlcpy(name, Name, MAX_TOPIC_SIZE);
}
void smartSwitch::set_output(uint8_t outpin, uint8_t intense)
{
    if (intense > 0) /* PWM OUTOUT defined by intense >0 */
    {
        _output_pwm = true;
        _pwm_ints = intense;
        _outputPin = outpin;
        pinMode(outpin, OUTPUT);

#if defined(ESP8266)
        analogWriteRange(1023);
#endif
    }
    else
    {
        if (outpin != UNDEF_PIN)
        {
            _outputPin = outpin;
            pinMode(outpin, OUTPUT);
        }
        else
        {
            _virtCMD = true;
        }
        _output_pwm = false;
    }
}

void smartSwitch::set_input(uint8_t inpin, uint8_t t)
{
    _button_type = t;
    if (_button_type == NO_INPUT || inpin == UNDEF_PIN)
    {
        _useButton = false;
    }
    else if (inpin != UNDEF_PIN && _button_type > NO_INPUT)
    {
        _useButton = true;
        _inputButton.begin(inpin);
        _inputButton.loop();
    }

    if (t == ON_OFF_SW)
    {
        _inputButton.setPressedHandler(std::bind(&smartSwitch::_OnOffSW_ON_handler, this, std::placeholders::_1));
        _inputButton.setReleasedHandler(std::bind(&smartSwitch::_OnOffSW_OFF_handler, this, std::placeholders::_1));
    }
    else if (t == MOMENTARY_SW)
    {
        _inputButton.setPressedHandler(std::bind(&smartSwitch::_toggle_handle, this, std::placeholders::_1));
    }
}
void smartSwitch::set_lockSW()
{
    _in_lockdown = true;
}
void smartSwitch::set_unlockSW()
{
    _in_lockdown = false;
}
void smartSwitch::set_useLockdown(bool t)
{
    _use_lockdown = t;
}
void smartSwitch::set_indiction(uint8_t pin, bool dir)
{
    if (pin != UNDEF_PIN)
    {
        _use_indic = true;
        _indicPin = pin;
        _indic_on = dir;
        pinMode(_indicPin, OUTPUT);
    }
}
void smartSwitch::init_lockdown()
{
    if (_use_lockdown)
    {
        _in_lockdown = true;
    }
}
void smartSwitch::release_lockdown()
{
    if (_use_lockdown)
    {
        _in_lockdown = false;
    }
}

void smartSwitch::turnON_cb(uint8_t type, unsigned int temp_TO)
{
    if (!_in_lockdown)
    {
        if (!_virtCMD)
        {
            if (!_output_pwm) /* NOT PWM output*/
            {
                if (!_isON())
                {
                    _HWon();
                }
                else
                {
                    Serial.println(" Already on");
                    return;
                }
            }
            else /* PWM output*/
            {
                if (!_PWM_ison)
                {
                    _HWon(_pwm_ints);
                }
                else
                {
                    Serial.println(" Already on");
                    return;
                }
            }

            if (temp_TO != 0)
            {
                _timeout_temp = temp_TO * 1000;
            }
            _start_timeout();
            unsigned long _t = 0;
            if (_use_timeout)
            {
                temp_TO == 0 ? _t = _timeout_duration : _t = _timeout_temp;
            }
            _update_telemetry(SW_ON, type, _t);
        }
        else
        {
            if (_guessState == SW_OFF)
            {
                _start_timeout();
                _guessState = !_guessState;
                _update_telemetry(SW_ON, type);
            }
        }
    }
}
void smartSwitch::turnOFF_cb(uint8_t type)
{
    if (!_in_lockdown)
    {
        if (!_virtCMD)
        {
            if (!_output_pwm)
            {
                if (_isON())
                {
                    _HWoff();
                    _stop_timeout();
                    _update_telemetry(SW_OFF, type, 0);
                }
                else
                {
                    Serial.println(" Already off");
                }
            }
            else
            {
                _HWoff();
                _stop_timeout();
                _update_telemetry(SW_OFF, type, 0);
            }
        }
        else
        {
            if (_guessState == SW_ON)
            {
                _stop_timeout();
                _guessState = !_guessState;
                _update_telemetry(SW_OFF, type);
            }
        }
    }
}
int smartSwitch::get_remain_time()
{
    if (_timeout_clk.isRunning() && _use_timeout)
    {
        return _timeout_temp == 0 ? _timeout_duration - _timeout_clk.elapsed() : _timeout_temp - _timeout_clk.elapsed();
    }
    else
    {
        return 0;
    }
}

uint8_t smartSwitch::get_SWstate()
{
    if (!_virtCMD)
    {
        return _isON();
    }
    else
    {
        return 255;
    }
}
void smartSwitch::get_SW_props(SW_props &props)
{
    props.id = _inputButton.getID();
    props.type = _button_type;
    props.inpin = _inputButton.getPin();
    props.outpin = _outputPin;
    props.timeout = _use_timeout;
    props.virtCMD = _virtCMD;
    props.lockdown = _use_lockdown;
    props.PWM = _output_pwm;
    props.name = name;
}
void smartSwitch::print_preferences()
{
    Serial.print("\n >>>>>> Switch #:");
    Serial.print(_inputButton.getID());
    Serial.println(" <<<<<< ");

    Serial.print("Output :\t");
    Serial.println(_virtCMD ? "Virutal" : "Relay");

    Serial.print("MQTT:\t\t");
    Serial.println(name);

    Serial.print("in_pins #:\t");
    Serial.println(_inputButton.getPin());

    Serial.print("input type:\t");
    Serial.println(_button_type);

    Serial.print("out_pin #:\t");
    Serial.println(_outputPin);

    Serial.print("PWM:\t\t");
    Serial.println(_output_pwm);

    Serial.print("use indic:\t");
    Serial.println(_use_indic);

    Serial.print("use timeout:\t");
    Serial.println(_use_timeout);

    Serial.print("use lockdown:\t");
    Serial.println(_use_lockdown ? "YES" : "NO");

    Serial.println(" >>>>>>>> END <<<<<<<< \n");
}

bool smartSwitch::loop()
{
    bool lckdown = (_use_lockdown && !_in_lockdown) || (!_use_lockdown);

    if (_useButton && lckdown)
    {
        _inputButton.loop();
    }
    if (_use_timeout && lckdown)
    {
        _timeout_loop();
    }
    if (_use_indic)
    {
        if (_isON())
        {
            _turn_indic_on();
        }
        else
        {
            _turn_indic_off();
        }
    }
    return telemtryMSG.newMSG;
}
void smartSwitch::clear_newMSG()
{
    telemtryMSG.newMSG = false;
}

bool smartSwitch::useTimeout()
{
    return _use_timeout;
}
bool smartSwitch::is_virtCMD()
{
    return _virtCMD;
}
bool smartSwitch::is_useButton()
{
    return _useButton;
}

bool smartSwitch::_isON()
{
    if (_output_pwm)
    {
        return _PWM_ison;
    }
    else
    {
        return (digitalRead(_outputPin) == OUTPUT_ON);
    }
}
void smartSwitch::_HWoff()
{
    if (_output_pwm)
    {
        analogWrite(_outputPin, 0);
        _PWM_ison = false;
    }
    else
    {
        digitalWrite(_outputPin, !OUTPUT_ON);
    }
}
void smartSwitch::_HWon(uint8_t val)
{
    if (_output_pwm)
    {
        int res = 0;
#if defined(ESP8266)
        res = 1023;
#elif defined(ESP32)
        res = 4097;
#endif
        analogWrite(_outputPin, (int)((res * val) / 100));
        _PWM_ison = true;
    }
    else
    {
        digitalWrite(_outputPin, OUTPUT_ON);
    }
}
void smartSwitch::_timeout_loop()
{
    if (_timeout_clk.isRunning())
    {
        if (_timeout_temp != 0 && _timeout_clk.hasPassed(_timeout_temp)) /* ad-hoc timeout*/
        {
            turnOFF_cb(SW_TIMEOUT);
        }
        else if (_timeout_temp == 0 && _timeout_clk.hasPassed(_timeout_duration)) /* preset timeout */
        {
            turnOFF_cb(SW_TIMEOUT);
        }
    }
}
void smartSwitch::_turn_indic_on()
{
    digitalWrite(_indicPin, _indic_on);
}
void smartSwitch::_turn_indic_off()
{
    digitalWrite(_indicPin, !_indic_on);
}
void smartSwitch::_stop_timeout()
{
    if (_use_timeout)
    {
        _timeout_clk.stop();
        _timeout_temp = 0;
    }
}
void smartSwitch::_start_timeout()
{
    if (_use_timeout)
    {
        _timeout_clk.stop();
        _timeout_clk.start();
    }
}
void smartSwitch::_update_telemetry(uint8_t state, uint8_t type, unsigned long te)
{
    telemtryMSG.newMSG = true;
    telemtryMSG.state = state;
    telemtryMSG.reason = type;
    telemtryMSG.clk_end = te;
}
void smartSwitch::_OnOffSW_Relay(uint8_t i, bool state, uint8_t type)
{
    if (state == true)
    {
        turnON_cb(type);
    }
    else
    {
        turnOFF_cb(type);
    }
}
void smartSwitch::_toggleRelay(uint8_t i, uint8_t type)
{
    if (!_virtCMD)
    {
        if (_isON())
        {
            turnOFF_cb(type);
        }
        else
        {
            turnON_cb(type);
        }
    }
    else
    {
        if (_guessState == true)
        {
            turnOFF_cb(type);
        }
        else
        {
            turnON_cb(type);
        }
    }
}

/* Button2 Handlers */
void smartSwitch::_OnOffSW_ON_handler(Button2 &b)
{
    _OnOffSW_Relay(b.getID(), OUTPUT_ON, BUTTON_INPUT);
}
void smartSwitch::_OnOffSW_OFF_handler(Button2 &b)
{
    _OnOffSW_Relay(b.getID(), !OUTPUT_ON, BUTTON_INPUT);
}
void smartSwitch::_toggle_handle(Button2 &b)
{
    _toggleRelay(b.getID(), BUTTON_INPUT);
}