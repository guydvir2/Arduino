#include <Arduino.h>
#include "smartSwitch.h"

smartSwitch::smartSwitch() : _inSW(1),
                             _timeout_clk(Chrono::MILLIS)
{
    _id = _next_id++;
}

void smartSwitch::set_id(uint8_t i)
{
    _id = i;
}
void smartSwitch::set_timeout(int t)
{
    if (t > 0)
    {
        _use_timeout = true;
        _timeout_duration = t * 1000;
        _stop_timeout();
    }
    else
    {
        _use_timeout = false;
    }
}
void smartSwitch::set_name(const char *Name)
{
    strlcpy(name, Name, MAX_TOPIC_SIZE);
}
void smartSwitch::set_output(uint8_t outpin, uint8_t intense, bool dir)
{
    OUTPUT_ON = dir;
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

void smartSwitch::set_input(uint8_t inpin, uint8_t t, bool dir)
{
    _button_type = t;
    BUTTON_PRESSED = dir;

    if (_button_type == NO_INPUT || inpin == UNDEF_PIN)
    {
        _useButton = false;
    }
    else if (inpin != UNDEF_PIN && _button_type > NO_INPUT)
    {
        _useButton = true;
        _inSW.set_debounce(50);

        _ez_sw_id = _inSW.add_switch(t, inpin, circuit_C2); /* pullup input */
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

void smartSwitch::turnON_cb(uint8_t type, unsigned int temp_TO, int intense)
{
    if (!_in_lockdown)
    {
        if (!_virtCMD)
        {
            if (!_output_pwm) /* NOT PWM output*/
            {
                if (!_isOUTPUT_ON())
                {
                    _setOUTPUT_ON();
                }
                else
                {
                    Serial.println(F("Already on"));
                    return;
                }
            }
            else /* PWM output*/
            {
                if (!_PWM_ison)
                {
                    if (intense != 0) /* Intersity was defined locally*/
                    {
                        _setOUTPUT_ON(intense);
                    }
                    else
                    {
                        _setOUTPUT_ON(_pwm_ints);
                    }
                }
                else
                {
                    Serial.println(F("Already on"));
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
                if (_isOUTPUT_ON())
                {
                    _setOUTPUT_OFF();
                    _stop_timeout();
                    _update_telemetry(SW_OFF, type, 0);
                }
                else
                {
                    Serial.println(F("Already off"));
                }
            }
            else
            {
                _setOUTPUT_OFF();
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
        return _isOUTPUT_ON();
    }
    else
    {
        return 255;
    }
}
void smartSwitch::get_SW_props(SW_props &props)
{
    props.id = _id;
    props.type = _button_type;
    props.inpin = _inSW.switches[_ez_sw_id].switch_pin;
    props.outpin = _outputPin;
    props.timeout = _use_timeout;
    props.virtCMD = _virtCMD;
    props.lockdown = _use_lockdown;
    props.PWM = _output_pwm;
    props.name = name;
}
void smartSwitch::print_preferences()
{
    Serial.print(F("\n >>>>>> Switch #"));
    Serial.print(_id);
    Serial.println(F(" <<<<<< "));

    Serial.print(F("Output Type :\t"));
    Serial.println(_virtCMD ? "Virtual" : "Real-Switch");
    Serial.print(F("Name:\t"));
    Serial.println(name);

    Serial.print(F("input type:\t"));
    Serial.print(_button_type);
    Serial.println(F(" ; 0:None; 1:Button, 2:Toggle"));
    Serial.print(F("input_pin:\t"));
    Serial.println(_inSW.switches[_ez_sw_id].switch_pin);
    Serial.print(F("outout_pin:\t"));
    Serial.println(_outputPin);
    Serial.print(F("isPWM:\t"));
    Serial.println(_output_pwm == 0 ? "No" : "Yes");
    Serial.print(F("use indic:\t"));
    Serial.println(_use_indic ? "Yes" : "No");
    if (_use_indic)
    {
        Serial.print(F("indic_Pin:\t"));
        Serial.println(_indicPin);
    }

    Serial.print(F("use timeout:\t"));
    Serial.println(_use_timeout ? "Yes" : "No");

    if (_timeout_duration > 0)
    {
        Serial.print(F("timeout [sec]:\t"));
        Serial.println(_timeout_duration / 1000);
    }

    Serial.print(F("use lockdown:\t"));
    Serial.println(_use_lockdown ? "YES" : "NO");

    Serial.println(F(" >>>>>>>> END <<<<<<<< \n"));
    Serial.flush();
}

bool smartSwitch::loop()
{
    bool lckdown = (_use_lockdown && !_in_lockdown) || (!_use_lockdown);

    if (_useButton && lckdown && _inSW.read_switch(_ez_sw_id) == switched) /* Input change*/
    {
        if (_inSW.switches[_ez_sw_id].switch_type == toggle_switch) /* For Toggle only */
        {
            if (_inSW.switches[_ez_sw_id].switch_status == !on && (get_SWstate() == 1 || (get_SWstate() == 255 && _guessState == SW_ON)))
            {
                turnOFF_cb(BUTTON_INPUT);
            }
            else if (_inSW.switches[_ez_sw_id].switch_status == on && (get_SWstate() == 0 || (get_SWstate() == 255 && _guessState == SW_OFF)))
            {
                turnON_cb(BUTTON_INPUT);
            }
            else
            {
                yield();
            }
        }
        else /* For Button only */
        {
            if (get_SWstate())
            {
                turnOFF_cb(BUTTON_INPUT);
            }
            else
            {
                turnON_cb(BUTTON_INPUT);
            }
        }
    }
    else if (_use_timeout && lckdown)
    {
        _timeout_loop();
    }
    if (_use_indic)
    {
        if (_isOUTPUT_ON())
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
uint8_t smartSwitch::_next_id = 0;
bool smartSwitch::_isOUTPUT_ON()
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
void smartSwitch::_setOUTPUT_OFF()
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
void smartSwitch::_setOUTPUT_ON(uint8_t val)
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
