#include <Arduino.h>
#include "smartSwitch.h"

smartSwitch::smartSwitch() : _inputButton(),
                             _timeout_clk(Chrono::MILLIS)
{
}
void smartSwitch::set_name(char *Name)
{
    strlcpy(name, Name, MAX_NAME_LEN);
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
    if (inpin != UNDEF_PIN)
    {
        _useButton = true;
        _inputButton.begin(inpin);
    }

    if (t == 1)
    {
        _inputButton.setPressedHandler(std::bind(&smartSwitch::_OnOffSW_ON_handler, this, std::placeholders::_1));
        _inputButton.setReleasedHandler(std::bind(&smartSwitch::_OnOffSW_OFF_handler, this, std::placeholders::_1));
    }
    else if (t == 2)
    {
        _inputButton.setPressedHandler(std::bind(&smartSwitch::_toggle_handle, this, std::placeholders::_1));
    }
}
void smartSwitch::set_timeout(int t)
{
    _use_timeout = true;
    _timeout_duration = t * 1000;
    _stop_timeout();
}
void smartSwitch::turnON_cb(uint8_t type)
{
    if (!_virtCMD)
    {
        if (!_isON())
        {
            HWturnON(_outputPin);
            _start_timeout();
            _update_telemetry(1, type);
        }
        else
        {
            Serial.println(" Already on");
        }
    }
    else
    {
        if (_guessState == false)
        {
            _start_timeout();
            _guessState = !_guessState;
            _update_telemetry(1, type);
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
            _update_telemetry(0,type);
        }
        else
        {
            Serial.println(" Already off");
        }
    }
    else
    {
        if (_guessState == true)
        {
            _stop_timeout();
            _guessState = !_guessState;
            _update_telemetry(0, type);
        }
    }
}
void smartSwitch::set_id(uint8_t i)
{
    _inputButton.setID(i);
}
void smartSwitch::get_prefences()
{
    Serial.print("<<<<<<< smartSwitch #");
    Serial.print(get_id());
    Serial.println(" >>>>>>>");
    Serial.print("> Type:\t\t");
    Serial.print(_button_type);
    Serial.println("  [0:None, 1:On-Off, 2:pushButton]");
    Serial.print("> Name:\t\t");
    Serial.println(strcmp(name, "") != 0 ? name : "None");

    Serial.print("> useInput:\t");
    Serial.println(get_inpin() != UNDEF_PIN ? "YES" : "NO");
    Serial.print("> useoutput:\t");
    Serial.println(_outputPin != UNDEF_PIN ? "YES" : "NO");
    Serial.print("> inputPin:\t");
    Serial.println(_inputButton.getPin());
    Serial.print("> outputPin:\t");
    Serial.println(_outputPin);

    Serial.print("> virtualCMD:\t");
    Serial.println(_virtCMD ? name : "NO");
    Serial.print("> MCU:\t\t");
    bool a = 0;
#if defined ESP32
    a = 1;
#elif
    a = 0;
#endif
    Serial.println(a == 0 ? "ESP8266" : "ESP32");

    Serial.print("<<<<<<< END ");
    Serial.println(">>>>>>");
}
void smartSwitch::get_telemetry(uint8_t state, uint8_t reason)
{
    state = telemtryMSG.state;
    reason = telemtryMSG.reason;
}
void smartSwitch::clear_newMSG()
{
    telemtryMSG.newMSG = false;
}
bool smartSwitch::loop()
{
    if (_useButton)
    {
        _inputButton.loop();
    }
    if (_use_timeout)
    {
        _timeout_loop();
    }
    return telemtryMSG.newMSG;
}
void smartSwitch::_timeout_loop()
{
    if (_timeout_clk.isRunning() & _timeout_clk.hasPassed(_timeout_duration))
    {
        turnOFF_cb(TIMEOUT);
    }
}
uint8_t smartSwitch::get_inpin()
{
    return _inputButton.getPin();
}
uint8_t smartSwitch::get_outpin()
{
    return _outputPin;
}
uint8_t smartSwitch::get_id()
{
    return _inputButton.getID();
}
int smartSwitch::get_remain_time()
{
    if (_timeout_clk.isRunning() && _use_timeout)
    {
        return _timeout_clk.elapsed();
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
bool smartSwitch::is_virtCMD()
{
    return _virtCMD;
}
bool smartSwitch::is_useButton()
{
    return _useButton;
}
void smartSwitch::_stop_timeout()
{
    if (_use_timeout)
    {
        _timeout_clk.stop();
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
uint8_t _next_id = 0;
bool smartSwitch::_isON()
{
    return (digitalRead(_outputPin) == OUTPUT_ON);
}
void smartSwitch::_update_telemetry(uint8_t state, uint8_t type)
{
    telemtryMSG.newMSG = true;
    telemtryMSG.state = state;
    telemtryMSG.reason = type;
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