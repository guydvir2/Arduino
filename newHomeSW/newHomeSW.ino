#include <myIOT2.h>
#include <Button2.h>
#include "myIOT_settings.h"

#define numSW 4
#define isPressed LOW
#define isON HIGH

const uint8_t buttonPins[] = {0, 4, 5, 12};
const uint8_t relayPins[] = {12, 13, 14, 16};

Button2 b0, b1, b2, b3;
Button2 *Buttons[] = {&b0, &b1, &b2, &b3};

void _turnON_cb(uint8_t i)
{
        if (digitalRead(i) == !isON)
        {
                digitalWrite(i, isON);
        }
        // Add MQTT notification
        // Add MQTT change state
}
void _turnOFF_cb(uint8_t i)
{
        if (digitalRead(i) == isON)
        {
                digitalWrite(i, !isON);
        }
        // Add MQTT notification
        // Add MQTT change state
}
void TurnRelay(uint8_t i, bool state)
{
        if (state == true)
        {
                _turnON_cb(i);
        }
        else
        {
                _turnOFF_cb(i);
        }
}
void toggleRelay(uint8_t i)
{
        if (digitalRead(i) == isON)
        {
                _turnOFF_cb(i);
        }
        else
        {
                _turnON_cb(i);
        }
}

void button_pressed_handle(Button2 &b)
{
        TurnRelay(b.getID(), isON);
}
void button_released_handle(Button2 &b)
{
        TurnRelay(b.getID(), !isON);
}
void init_buttons()
{
        for (byte i = 0; i < numSW; i++)
        {
                Buttons[i]->setID(i);
                Buttons[i]->begin(relayPins[i]);
                Buttons[i]->setPressedHandler(button_pressed_handle);
                Buttons[i]->setReleasedHandler(button_released_handle);
        }
}
void init_outputs()
{
        for (byte i = 0; i < numSW; i++)
        {
                pinMode(relayPins[i], OUTPUT);
        }
}

void setup()
{
        Serial.begin(115200);
        startIOTservices();
        init_buttons();
        init_outputs();
}
void loop()
{
        iot.looper();
}
