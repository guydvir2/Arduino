#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/xSW"
#define ADD_MQTT_FUNC addiotnalMQTT
#define VER "SONOFFsw_2.0"
//####################################################

// state definitions
#define RelayOn HIGH
#define SwitchOn LOW

// GPIO Pins for SONOFF BASIC
const int inputPin = 14;
const int inputLocalPin = 0;
const int outputPin = 12;
const int ledPin = 13;
//##########################

// GPIO status flags
bool outputPin_currentState;
bool inputPin_lastState;
bool inputLocalPin_lastState;
bool inputPin_currentState;
bool inputLocalPin_currentState;
// ###########################


// manual RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 3; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
// ####################

const int deBounceInt = 50;
bool OnAtBoot = true;

myIOT iot(DEVICE_TOPIC);

void setup() {
        startGPIOs();
        if (OnAtBoot == true ) {
                digitalWrite(outputPin,RelayOn);
        }
        else {
                digitalWrite(outputPin,!RelayOn);
        }

        iot.start_services(ADD_MQTT_FUNC);
        iot.startOTA();
}
void startGPIOs() {
        pinMode(inputPin, INPUT_PULLUP);
        pinMode(inputLocalPin, INPUT_PULLUP);
        pinMode(outputPin, OUTPUT);
        pinMode(ledPin, OUTPUT);

        inputLocalPin_lastState = digitalRead(inputLocalPin);
        inputPin_lastState = digitalRead(inputPin);
}
void addiotnalMQTT(char incoming_msg[50]){
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                if (outputPin_currentState == RelayOn) {
                        sprintf(state, "On");
                }
                else if (outputPin_currentState == !RelayOn ) {
                        sprintf(state, "Off");
                }

                // switch state
                if (inputPin_lastState == !RelayOn ) {
                        sprintf(state2, "OFF");
                }
                else if (inputPin_lastState == RelayOn) {
                        sprintf(state2, "ON");
                }
                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0 || strcmp(incoming_msg, "off") == 0) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s]", VER,iot.ver);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                iot.pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "flicker") == 0 ) {
                iot.pub_msg("Flicker");
                switchIt("MQTT", "on");
                delay(100);
                switchIt("MQTT", "off");
                delay(100);
                switchIt("MQTT", "on");
                delay(100);
                switchIt("MQTT", "off");
                delay(100);
        }
}
void PBit() {
        int pause = 2 * 5 * deBounceInt;
        digitalWrite(outputPin, RelayOn);
        delay(pause);
        digitalWrite(outputPin, !RelayOn);
        delay(pause);
}
void switchIt(char *type, char *dir) {
        char mqttmsg[50];

        if (strcmp(dir, "on") == 0) {
                digitalWrite(outputPin,RelayOn);
        }
        else if (strcmp(dir, "off") == 0) {
                digitalWrite(outputPin,!RelayOn);
        }

        if (iot.mqttConnected == true) {
                iot.pub_state(dir);
                sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
                iot.pub_msg(mqttmsg);
        }
}
void checkRemoteInput() {
        if (digitalRead(inputPin) != inputPin_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputPin) != inputPin_lastState) {
                        if (digitalRead(inputPin) == SwitchOn) {
                                switchIt("Button", "on");
                        }
                        else  {
                                switchIt("Button", "off");
                        }
                        inputPin_lastState = digitalRead(inputPin);

                }
                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "Bounce: cRead [%d] lRead[%d]", digitalRead(inputPin), inputPin_lastState);
                        iot.pub_msg(tMsg);
                }
        }
}
void checkLocalInput() {
        if (digitalRead(inputLocalPin) == SwitchOn) {
                delay(deBounceInt);
                if (digitalRead(inputLocalPin) == SwitchOn ) {
                        if (inputLocalPin_lastState == SwitchOn) {
                                switchIt("localButton", "on");
                        }
                        else  {
                                switchIt("localButton", "off");
                        }
                        inputLocalPin_lastState = digitalRead(inputLocalPin);
                        while (digitalRead(inputLocalPin) == SwitchOn) {
                                delay(50);
                        }
                }
        }
}
void readGpioStates() {
        outputPin_currentState = digitalRead(outputPin);
        inputLocalPin_currentState = digitalRead(inputLocalPin);
        inputPin_currentState = digitalRead(inputPin);
}

void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        readGpioStates();
        digitalWrite(ledPin, !digitalRead(outputPin));

        // react to commands (MQTT or local switch)
        checkRemoteInput();
        checkLocalInput();

        delay(50);
}
