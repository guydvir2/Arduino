#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW
#define HWturnON(i) digitalWrite(i, OUTPUT_ON)
#define HWturnOFF(i) digitalWrite(i, !OUTPUT_ON)

extern myIOT2 iot;
char *turnTypes[] = {"MQTT", "Button", "Remote"};

enum OPerTypes : const uint8_t
{
    _MQTT,
    _BUTTON,
    RF
};
void _gen_ButtMSG(uint8_t i, uint8_t type, bool request);

bool _isON(uint8_t i)
{
    return digitalRead(SW_v[i]->outPin) == OUTPUT_ON;
}
void _turnON_cb(uint8_t i, uint8_t type)
{
    if (!SW_v[i]->virtCMD)
    {
        if (!_isON(i))
        {
            HWturnON(SW_v[i]->outPin);
            _gen_ButtMSG(i, type, HIGH);
        }
        else
        {
            Serial.print(i);
            Serial.println(" Already on");
        }
    }
    else
    {
        iot.pub_noTopic(buttMQTTcmds[0], SW_v[i]->Topic);
        _gen_ButtMSG(i, type, HIGH);
    }
}
void _turnOFF_cb(uint8_t i, uint8_t type)
{
    if (!SW_v[i]->virtCMD)
    {
        if (_isON(i))
        {
            HWturnOFF(SW_v[i]->outPin);
            _gen_ButtMSG(i, type, LOW);
        }
        else
        {
            Serial.print(i);
            Serial.println(" Already off");
        }
    }
    else
    {
        iot.pub_noTopic(buttMQTTcmds[1], SW_v[i]->Topic);
        _gen_ButtMSG(i, type, LOW);
    }
}
void OnOffSW_Relay(uint8_t i, bool state, uint8_t type)
{
    if (state == true)
    {
        _turnON_cb(i, type);
    }
    else
    {
        _turnOFF_cb(i, type);
    }
}
void toggleRelay(uint8_t i, uint8_t type)
{
    if (!SW_v[i]->virtCMD)
    {
        if (_isON(i))
        {
            _turnOFF_cb(i, type);
        }
        else
        {
            _turnON_cb(i, type);
        }
    }
    else
    {
        if (SW_v[i]->guessState == true)
        {
            _turnOFF_cb(i, type);
        }
        else{
            _turnON_cb(i, type);
        }
        SW_v[i]->guessState = !SW_v[i]->guessState;
    }
}

/* ****************** Button2 Handlers **********************/
void OnOffSW_ON_handler(Button2 &b)
{
    OnOffSW_Relay(b.getID(), OUTPUT_ON, 1);
}
void OnOffSW_OFF_handler(Button2 &b)
{
    OnOffSW_Relay(b.getID(), !OUTPUT_ON, 1);
}
void toggle_handle(Button2 &b)
{
    toggleRelay(b.getID(), 1);
}