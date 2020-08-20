#include <myIOT.h>
#include "win_param.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "NodeMCU_6.0"
#define USE_BOUNCE_DEBUG false
#define RelayOn LOW
#define SwitchOn LOW


bool auto_relay_off;
int auto_relay_off_timeout;

//~~~~Internal Switch ~~~~~~
int inputUpPin;
int inputDownPin;
int outputUpPin;
int outputDownPin;
//~~~~External Input ~~~~~~~~~
int inputUpExtPin;
int inputDownExtPin;

// GPIO status flags
//~~~~Internal Switch ~~~~~~
bool inputUp_lastState;
bool inputDown_lastState;
//~~~~External Input ~~~~~~~
bool inputUpExt_lastState;
bool inputDownExt_lastState;

// state definitions & constatns

const int deBounceInt = 50;

// ~~ myIOT definitions ~~
#define ADD_MQTT_FUNC addiotnalMQTT
bool ext_inputs;
myIOT iot;


// ~~~~~~~~~~~~~~~~~~~~~~~

void startIOTservices()
{
        iot.useSerial = paramJSON["useSerial"];
        iot.useWDT = paramJSON["useWDT"];
        iot.useOTA = paramJSON["useOTA"];
        iot.useResetKeeper = paramJSON["useResetKeeper"];
        iot.resetFailNTP = paramJSON["useFailNTP"];
        iot.useDebug = paramJSON["useDebugLog"];
        iot.debug_level = 0; //All operations are monitored
        strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
        strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
        strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
        iot.start_services(ADD_MQTT_FUNC); //, SSID_ID, PASS_WIFI, MQTT_USER, MQTT_PASS, "192.168.3.201");
}
void setup()
{
        read_parameters_from_file();
        startGPIOs();
        startIOTservices();
        paramJSON.clear();
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs()
{
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        if (ext_inputs)
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
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                sprintf(msg, "ver2:[%s], AutoOFF[%d]", VER, auto_relay_off);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
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
        if (ext_inputs)
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

        if (auto_relay_off)
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
        if (ext_inputs)
        {
                checkSwitch_looper(inputUpExtPin, "up", inputUpExt_lastState, "extButton");
                checkSwitch_looper(inputDownExtPin, "down", inputDownExt_lastState, "extButton");
        }
        if (auto_relay_off)
        {
                checkTimeout_AutoRelay_Off(auto_relay_off_timeout);
        }
        delay(100);
}
