#include <myIOT2.h>
#include <Button2.h>
#include <RCSwitch.h>
#include "defs.h"

/* ~~~~~~~~~~ FOR DEBUG ONLY ~~~~~~~~~~~~~~~~~~~ */
#define USE_RF true
#define USE_BUTTONS true
int counter = 0;
long lastheap = 0;

#if USE_RF
RCSwitch RFreader = RCSwitch();
#endif
#if USE_BUTTONS
Button2 *Buttons[MAX_Relays] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
#endif

void BIT_outputs()
{
        for (uint8_t y = 0; y < numSW; y++)
        {
                OnOffSW_Relay(y, true, 2);
                delay(1000);
                OnOffSW_Relay(y, false, 2);
        }
}
void check_answer_mqtt()
{
        static unsigned long lastentry = 0;
        static unsigned long lastentry2 = 0;

        if (millis() - lastentry > 200)
        {
                pinMode(2, OUTPUT);
                digitalWrite(2, !digitalRead(2));
                lastentry = millis();
        }
        delay(50);
        if (millis() - lastentry2 > 1000)
        {
                long theap = ESP.getFreeHeap();
                if (abs(lastheap - theap) > 200)
                {
                        Serial.print("Current: ");
                        Serial.print((float)(theap / 1000.0));
                        Serial.print("kb");
                        Serial.print("\t Change:");
                        Serial.print((float)(lastheap - theap) / 1000);
                        Serial.println(" kb");
                        lastheap = theap;
                }
                lastentry2 = millis();
        }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#include "myIOT_settings.h"

bool _isON(uint8_t i)
{
        return digitalRead(i) == OUTPUT_ON;
}
void _pub_turn(uint8_t i, uint8_t type, bool request)
{
        char msg[50];
        sprintf(msg, "[%s]: [SW#%d][%s] Turn [%s]", turnTypes[type], i, ButtonNames[i], request == HIGH ? "ON" : "OFF");
        iot.pub_msg(msg);
        updateState(i, (int)request);
}
void _turnON_cb(uint8_t i, uint8_t type)
{
        if (!_isON(relayPins[i]))
        {

                turnON(relayPins[i]);
                _pub_turn(i, type, HIGH);
        }
        else
        {
                Serial.print(i);
                Serial.println(" Already on");
        }
        Serial.print("Counter #");
        Serial.println(++counter);
}
void _turnOFF_cb(uint8_t i, uint8_t type)
{
        if (_isON(relayPins[i]))
        {
                turnOFF(relayPins[i]);
                _pub_turn(i, type, LOW);
        }
        else
        {
                Serial.print(i);
                Serial.println(" Already off");
        }
        Serial.print("Counter #");
        Serial.println(++counter);
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

#if USE_BUTTONS
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
#endif

void init_buttons()
{
#if USE_BUTTONS
        if (useButton)
        {
                for (uint8_t i = 0; i < numSW; i++)
                {
                        Buttons[i] = new Button2;
                        Buttons[i]->begin(buttonPins[i]);
                        if (buttonTypes[i] == 0) /* On-Off Switch */
                        {
                                Buttons[i]->setPressedHandler(OnOffSW_ON_handler);
                                Buttons[i]->setReleasedHandler(OnOffSW_OFF_handler);
                        }
                        else if (buttonTypes[i] == 1) /* Momentary press */
                        {
                                Buttons[i]->setPressedHandler(toggle_handle);
                        }
                        Buttons[i]->setID(i);
                }
        }
#endif
}
void init_outputs()
{
        for (byte i = 0; i < numSW; i++)
        {
                pinMode(relayPins[i], OUTPUT);
                digitalWrite(relayPins[i], !OUTPUT_ON);
        }
}
void init_RF()
{
#if USE_RF
        if (useRF)
        {
                RFreader.enableReceive(RFpin);
        }
#endif
}
void loop_buttons()
{
#if USE_BUTTONS
        if (useButton)
        {
                for (byte i = 0; i < numSW; i++)
                {
                        Buttons[i]->loop();
                }
        }
#endif
}
void loop_RF()
{
#if USE_RF
        if (useRF)
        {
                if (RFreader.available())
                {
                        // sprintf(temp, "Received %d / %dbit Protocol: ", RFreader.getReceivedValue(), RFreader.getReceivedBitlength(), RFreader.getReceivedProtocol());
                        for (uint8_t i = 0; i < sizeof(KB_codes) / sizeof(KB_codes[0]); i++)
                        {
                                if (KB_codes[i] == RFreader.getReceivedValue())
                                {
                                        toggleRelay(i, RF);
                                        delay(500); /* To avoid bursts */
                                }
                        }
                        RFreader.resetAvailable();
                }
        }
#endif
}

void setup()
{
        startIOTservices();
        init_buttons();
        init_outputs();
        init_RF();
}
void loop()
{
        iot.looper();
        loop_buttons();
        loop_RF();
        // check_answer_mqtt();
}
