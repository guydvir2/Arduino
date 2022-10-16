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
    // strlcpy(name, Name, MAX_NAME_LEN);
}
void smartSwitch::set_output(uint8_t outpin)
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
}
void smartSwitch::set_input(uint8_t inpin, uint8_t t)
{
    _button_type = t;
    if (_button_type == 0 || inpin == UNDEF_PIN)
    {
        _useButton = false;
    }
    else if (inpin != UNDEF_PIN && _button_type > 0)
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
    if (!_virtCMD)
    {
        if (!_isON())
        {
            HWturnON(_outputPin);
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
            Serial.println(" Already on");
        }
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
void smartSwitch::turnOFF_cb(uint8_t type)
{
    if (!_virtCMD)
    {
        if (_isON())
        {
            HWturnOFF(_outputPin);
            _stop_timeout();
            _update_telemetry(SW_OFF, type, 0);

            // telemtryMSG.newMSG = true;
            // telemtryMSG.state = 0;
            // telemtryMSG.reason = type;
            // telemtryMSG.clk_end = 0;
        }
        else
        {
            Serial.println(" Already off");
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
void smartSwitch::get_prefences()
{
//     Serial.print("<<<<<<< smartSwitch #");
//     Serial.print(_inputButton.getID());
//     Serial.println(" >>>>>>>");
//     Serial.print("> Type:\t\t");
//     Serial.print(_button_type);
//     Serial.println("  [0:None, 1:On-Off, 2:pushButton]");
//     Serial.print("> Name:\t\t");
//     Serial.println(strcmp(name, "") != 0 ? name : "None");

//     Serial.print("> useInput:\t");
//     Serial.println(_inputButton.getPin() != UNDEF_PIN ? "YES" : "NO");
//     Serial.print("> useoutput:\t");
//     Serial.println(_outputPin != UNDEF_PIN ? "YES" : "NO");
//     Serial.print("> inputPin:\t");
//     Serial.println(_inputButton.getPin());
//     Serial.print("> outputPin:\t");
//     Serial.println(_outputPin);

//     Serial.print("> virtualCMD:\t");
//     Serial.println(_virtCMD ? name : "NO");
//     Serial.print("> MCU:\t\t");
//     bool a = 0;
// #if defined ESP32
//     a = 1;
// #elif defined ESP8266
//     a = 0;
// #endif
//     Serial.println(a == 0 ? "ESP8266" : "ESP32");

//     Serial.print("<<<<<<< END ");
//     Serial.println(">>>>>>");
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
    // props.name = name;
}

bool smartSwitch::loop()
{
    bool lckdown = true; //((_use_lockdown && !_in_lockdown) || (!_use_lockdown));

    if (_useButton && lckdown)
    {
        _inputButton.loop();
    }
    if (_use_timeout && lckdown)
    {
        _timeout_loop();
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
    return (digitalRead(_outputPin) == OUTPUT_ON);
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

    // Serial.println("Tele updated");

    // Serial.println("AT LIB");
    // Serial.print("state: ");
    // Serial.println(telemtryMSG.state);
    // Serial.print("reason: ");
    // Serial.println(telemtryMSG.reason);
    // Serial.print("clk_end: ");
    // Serial.println(telemtryMSG.clk_end);
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