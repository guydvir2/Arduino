#include "smartSwitch.h"

smartSwitch::smartSwitch() : _inputButton()
{
}
void smartSwitch::set_name(char *Name)
{
    strlcpy(name, Name, MAX_NAME_LEN);
}
void smartSwitch::set_output(uint8_t outpin)
{
    if (outpin != 255)
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
    if (inpin != 255)
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
void smartSwitch::turnON_cb(uint8_t type)
{
    if (!_virtCMD)
    {
        if (!_isON())
        {
            HWturnON(_outputPin);
            Serial.println("ON");
            // _gen_ButtMSG(i, type, HIGH);
            telemtryMSG.newMSG = true;
            telemtryMSG.state = 1;
            telemtryMSG.reason = type;
        }
        else
        {
            // Serial.print(i);
            Serial.println(" Already on");
        }
    }
    else
    {
        // iot.pub_noTopic(buttMQTTcmds[0], SW_v[i]->Topic);
        // _gen_ButtMSG(i, type, HIGH);
        set_extON();
        telemtryMSG.newMSG = true;
        telemtryMSG.state = 1;
        telemtryMSG.reason = type;
        Serial.println("O_VIRTN");
    }
}
void smartSwitch::turnOFF_cb(uint8_t type)
{
    if (!_virtCMD)
    {
        if (_isON())
        {
            HWturnOFF(_outputPin);
            // _gen_ButtMSG(i, type, LOW);
            Serial.println("OFF");
            telemtryMSG.newMSG = true;
            telemtryMSG.state = 0;
            telemtryMSG.reason = type;
        }
        else
        {
            // Serial.print(i);
            Serial.println(" Already off");
        }
    }
    else
    {
        // iot.pub_noTopic(buttMQTTcmds[1], SW_v[i]->Topic);
        // _gen_ButtMSG(i, type, LOW);
        // set_extOFF();
        Serial.println("VIRT_OFF");
        telemtryMSG.newMSG = true;
        telemtryMSG.state = 0;
        telemtryMSG.reason = type;
    }
}
void smartSwitch::set_id(uint8_t i)
{
    _id = i;
}
void smartSwitch::get_prefences()
{
    Serial.print("<<<<<<< smartSwitch #");
    Serial.print(_id);
    Serial.println(" >>>>>>>");
    Serial.print("> Type:\t");
    Serial.print(_button_type);
    Serial.println("  [0:None, 1:On-Off, 2:pushButton]");
    Serial.print("> Name:\t");
    Serial.println(strcmp(name, "") != 0 ? name : "None");

    Serial.print("> useInput:\t");
    Serial.println(get_inpin() != 255 ? "YES" : "NO");
    Serial.print("> useoutput:\t");
    Serial.println(_outputPin != 255 ? "YES" : "NO");
    Serial.print("> virtualCMD:\t");
    Serial.println(_virtCMD ? name : "NO");

    Serial.print("<<<<<<< END ");
    Serial.println(">>>>>>");
}
void smartSwitch::get_telemetry(uint8_t state, uint8_t i)
{
}
void smartSwitch::loop()
{
    if (_useButton)
    {
        _inputButton.loop();
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
    return _id;
}
bool smartSwitch::is_virtCMD()
{
    return _virtCMD;
}
bool smartSwitch::is_useButton()
{
    return _useButton;
}

uint8_t _next_id = 0;
bool smartSwitch::_isON()
{
    return (digitalRead(_outputPin) == OUTPUT_ON);
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
        _guessState = !_guessState;
    }
}

/* Button2 Handlers */
void smartSwitch::_OnOffSW_ON_handler(Button2 &b)
{
    _OnOffSW_Relay(b.getID(), OUTPUT_ON, _MQTT);
}
void smartSwitch::_OnOffSW_OFF_handler(Button2 &b)
{
    _OnOffSW_Relay(b.getID(), !OUTPUT_ON, _MQTT);
}
void smartSwitch::_toggle_handle(Button2 &b)
{
    _toggleRelay(b.getID(), _MQTT);
}