#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "NodeMCU_5.8"
#define USE_BOUNCE_DEBUG false
#define USE_2_EXT_INPUT false // Only for dual input window
#define USE_AUTO_RELAY_OFF true
// #define USE_AUTO_OFF true
#define AUTO_RELAY_TIMEOUT 90
// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false
#define USE_WDT true
#define USE_OTA true
#define USE_RESETKEEPER false
#define USE_FAILNTP true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "!@#" // laundry & saloonSingle has different GPIO from others
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "Windows"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// state definitions
#define RelayOn LOW
#define SwitchOn LOW

// GPIO Pins for ESP8266
//~~~~Internal Switch ~~~~~~
const int inputUpPin = D1;   // main is D1 // D3 only for saloonSingle
const int inputDownPin = D2; // main is D2 // D3 only for laundryRoom 
const int outputUpPin = 14;
const int outputDownPin = 12;
//~~~~External Input ~~~~~~~~~
const int inputUpExtPin = 0;
const int inputDownExtPin = 2;
//############################

// GPIO status flags
//~~~~Internal Switch ~~~~~~
bool inputUp_lastState;
bool inputDown_lastState;
//~~~~External Input ~~~~~~~
bool inputUpExt_lastState;
bool inputDownExt_lastState;
// ###########################

const int deBounceInt = 50;

void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC); //, SSID_ID, PASS_WIFI, MQTT_USER, MQTT_PASS, "192.168.3.201");
}
void setup()
{
        startGPIOs();
        startIOTservices();
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs()
{
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        if (USE_2_EXT_INPUT)
        {
                pinMode(inputUpExtPin, INPUT_PULLUP);
                pinMode(inputDownExtPin, INPUT_PULLUP);
        }

        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        allOff();
}
void addiotnalMQTT(char incoming_msg[50])
{
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0)
        {
                // relays state
                if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn)
                {
                        sprintf(state, "invalid Relay State");
                }
                else if (digitalRead(outputUpPin) == !RelayOn && digitalRead(outputDownPin) == RelayOn)
                {
                        sprintf(state, "DOWN");
                }
                else if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == !RelayOn)
                {
                        sprintf(state, "UP");
                }
                else
                {
                        sprintf(state, "OFF");
                }

                // switch state
                if (inputUp_lastState == !RelayOn && inputDown_lastState == !RelayOn)
                {
                        sprintf(state2, "OFF");
                }
                else if (inputUp_lastState == RelayOn && inputDown_lastState == !RelayOn)
                {
                        sprintf(state2, "UP");
                }
                else if (inputUp_lastState == !RelayOn && inputDown_lastState == RelayOn)
                {
                        sprintf(state2, "DOWN");
                }
                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0)
        {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0)
        {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d],ResetKeeper[%d], FailNTP[%d], AutoOFF[%d]",
                        VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP, USE_AUTO_RELAY_OFF);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0)
        {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [up, down, off]");
                iot.pub_msg(msg);
        }
}
// ~~~~ maintability ~~~~~~
void allOff()
{
        digitalWrite(outputUpPin, !RelayOn);
        digitalWrite(outputDownPin, !RelayOn);
        inputUp_lastState = digitalRead(inputUpPin);
        inputDown_lastState = digitalRead(inputDownPin);
        if (USE_2_EXT_INPUT)
        {
                inputUpExt_lastState = digitalRead(inputUpExtPin);
                inputDownExt_lastState = digitalRead(inputDownExtPin);
        }
}

unsigned long autoOff_clock = 0;
void checkTimeout_AutoRelay_Off(int timeout_off)
{
        if (autoOff_clock != 0 && millis() - autoOff_clock > timeout_off * 1000)
        {
                switchIt("timeout", "off");
                autoOff_clock = 0;
        }
}
// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir)
{
        char mqttmsg[50];
        bool states[2];

        if (strcmp(dir, "up") == 0)
        {
                states[0] = RelayOn;
                states[1] = !RelayOn;
        }
        else if (strcmp(dir, "down") == 0)
        {
                states[0] = !RelayOn;
                states[1] = RelayOn;
        }
        else if (strcmp(dir, "off") == 0)
        {
                states[0] = !RelayOn;
                states[1] = !RelayOn;
        }

        bool Up_read = digitalRead(outputUpPin);
        bool Down_read = digitalRead(outputDownPin);

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (Up_read != states[0] && Down_read != states[1])
        {
                allOff();
                delay(deBounceInt * 2);
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (Up_read != states[0] || Down_read != states[1])
        {
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        iot.pub_state(dir);
        sprintf(mqttmsg, "%s: Switched [%s]", type, dir);
        iot.pub_msg(mqttmsg);

        if (USE_AUTO_RELAY_OFF)
        {
                if (digitalRead(outputDownPin) == RelayOn || digitalRead(outputUpPin) == RelayOn)
                {
                        autoOff_clock = millis();
                }
                else
                {
                        // when switched off, cancel timeout
                        autoOff_clock = 0;
                }
        }
}
void checkSwitch_looper(const int &pin, char *dir, bool &lastState, char *type = "Button")
{
        if (digitalRead(pin) != lastState)
        {
                delay(deBounceInt);
                if (digitalRead(pin) != lastState)
                {
                        if (digitalRead(pin) == SwitchOn)
                        {
                                switchIt(type, dir);
                                lastState = digitalRead(pin);
                        }
                        else if (digitalRead(pin) == !SwitchOn)
                        {
                                switchIt(type, "off");
                                lastState = digitalRead(pin);
                        }
                }
                else if (USE_BOUNCE_DEBUG)
                { // for debug only
                        char tMsg[100];
                        sprintf(tMsg, "[%s] Bounce: cRead[%d] lRead[%d]", dir, digitalRead(pin), lastState);
                        iot.pub_log(tMsg);
                }
        }
}
void verifyNotHazardState()
{
        if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn)
        {
                switchIt("Button", "off");
                iot.sendReset("HazradState");
        }
}

void loop()
{
        iot.looper();
        verifyNotHazardState(); // both up and down are ---> OFF

        checkSwitch_looper(inputUpPin, "up", inputUp_lastState, "inButton");
        checkSwitch_looper(inputDownPin, "down", inputDown_lastState, "inButton");
        if (USE_2_EXT_INPUT)
        {
                checkSwitch_looper(inputUpExtPin, "up", inputUpExt_lastState, "extButton");
                checkSwitch_looper(inputDownExtPin, "down", inputDownExt_lastState, "extButton");
        }
        if (USE_AUTO_RELAY_OFF)
        {
                checkTimeout_AutoRelay_Off(AUTO_RELAY_TIMEOUT);
        }
        delay(100);
}
