// #include <myIOT2.h>
#include <Button2.h>
#include <RCSwitch.h>
#include "defs.h"

#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

RCSwitch RFreader = RCSwitch();
Button2 *Buttons[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

const char *mqtt_server = "192.168.2.100";

//
// #include "myIOT_settings.h"

void setup_wifi()
{

        delay(10);
        // We start by connecting to a WiFi network
        Serial.println();
        Serial.print("Connecting to ");
        // Serial.println(ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin("dvirz_iot", "GdSd13100301");

        while (WiFi.status() != WL_CONNECTED)
        {
                delay(500);
                Serial.print(".");
        }

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++)
        {
                Serial.print((char)payload[i]);
        }
        Serial.println();

        // Switch on the LED if an 1 was received as first character
}

void reconnect()
{
        // Loop until we're reconnected
        while (!client.connected())
        {
                Serial.print("Attempting MQTT connection...");
                // Create a random client ID
                String clientId = "ESP8266Client-";
                clientId += String(random(0xffff), HEX);
                // Attempt to connect
                if (client.connect(clientId.c_str()),"guy","kupelu9e")
                {
                        Serial.println("connected");
                        // Once connected, publish an announcement...
                        client.publish("myHome/log", "hello world");
                        // ... and resubscribe
                        client.subscribe("myHome/test");
                }
                else
                {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 5 seconds");
                        // Wait 5 seconds before retrying
                        delay(5000);
                }
        }
}

void _turnON_cb(uint8_t i, uint8_t type)
{
        if (digitalRead(relayPins[i]) == !OUTPUT_ON)
        {
                char msg[50];
                turnON(relayPins[i]);
                sprintf(msg, "[%s]: [%s] Turn [%s]", turnTypes[type], ButtonNames[i], "ON");
                // iot.pub_msg(msg);
                // updateState(i, 1);
        }
        else
        {
                Serial.print(i);
                Serial.println(" Already on");
        }
}
void _turnOFF_cb(uint8_t i, uint8_t type)
{
        if (digitalRead(relayPins[i]) == OUTPUT_ON)
        {
                char msg[50];
                turnOFF(relayPins[i]);
                sprintf(msg, "[%s]: [%s] Turn [%s]", turnTypes[type], ButtonNames[i], "OFF");
                // iot.pub_msg(msg);
                // updateState(i, 0);
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
        if (digitalRead(relayPins[i]) == OUTPUT_ON)
        {
                _turnOFF_cb(i, type);
        }
        else
        {
                _turnON_cb(i, type);
        }
}

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

void init_buttons()
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
void init_outputs()
{
        for (byte i = 0; i < numSW; i++)
        {
                pinMode(relayPins[i], OUTPUT);
        }
}
void init_RF()
{
        // RFreader.enableReceive(RFpin);
}
void loop_buttons()
{
        for (byte i = 0; i < numSW; i++)
        {
                Buttons[i]->loop();
        }
}
void loop_RF()
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

void BIT_outputs()
{
        for (uint8_t y = 0; y < numSW; y++)
        {
                OnOffSW_Relay(y, true, 2);
                delay(1000);
                OnOffSW_Relay(y, false, 2);
        }
}

void setup()
{
        // startIOTservices();
        Serial.begin(115200);
        setup_wifi();
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
        reconnect();
        init_buttons();
        init_outputs();
        // init_RF();
}
void loop()
{
        // iot.looper();
        loop_buttons();
        // loop_RF();
        if (!client.connected())
        {
                reconnect();
        }
        client.loop();
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
                Serial.println(ESP.getFreeHeap());
                lastentry2 = millis();
        }
}
