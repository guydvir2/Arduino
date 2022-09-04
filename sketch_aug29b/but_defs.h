#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW

#define turnON(i) digitalWrite(i, OUTPUT_ON)
#define turnOFF(i) digitalWrite(i, !OUTPUT_ON)

char *turnTypes[] = {"MQTT", "Button", "Remote"};

enum OPerTypes : const uint8_t
{
    _MQTT,
    _BUTTON,
    RF
};

bool _isON(uint8_t i)
{
    return digitalRead(i) == OUTPUT_ON;
}
void _gen_ButtMSG(uint8_t i, uint8_t type, bool request);

void _turnON_cb(uint8_t i, uint8_t type)
{
    if (!_isON(relayPins[i]))
    {
        turnON(relayPins[i]);
        _gen_ButtMSG(i, type, HIGH);
    }
    else
    {
        Serial.print(i);
        Serial.println(" Already on");
    }
}
void _turnOFF_cb(uint8_t i, uint8_t type)
{
    if (_isON(relayPins[i]))
    {
        turnOFF(relayPins[i]);
        _gen_ButtMSG(i, type, LOW);
    }
    else
    {
        Serial.print(i);
        Serial.println(" Already off");
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
    if (_isON(relayPins[i]))
    {
        _turnOFF_cb(i, type);
    }
    else
    {
        _turnON_cb(i, type);
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