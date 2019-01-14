#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Windows/Box_5"
#define ADD_MQTT_FUNC addiotnalMQTT
#define VER "3.25"
//####################################################

// state definitions
#define RelayOn LOW
#define SwitchOn LOW

// GPIO Pins for ESP8266
const int inputUpPin = 4;
const int inputDownPin = 5;
const int outputUpPin = 14;
const int outputDownPin = 12;
//##########################

// GPIO status flags
bool outputUp_currentState;
bool outputDown_currentState;
bool inputUp_lastState;
bool inputDown_lastState;
bool inputUp_currentState;
bool inputDown_currentState;
// ###########################

// manual RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 3; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
const int timeInterval_resetPress = 1500; // time between consq presses to init RESET cmd
// ####################

const int deBounceInt = 50;
// bool useSerial = true;

myIOT iot(DEVICE_TOPIC);

void setup() {
        startGPIOs();

        iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker
        iot.startOTA();
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs() {
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        allOff();
}
void PBit() {
        int pause = 2 * 5 * deBounceInt;
        allOff();
        delay(pause);
        digitalWrite(outputUpPin, RelayOn);
        delay(pause);
        digitalWrite(outputUpPin, !RelayOn);
        delay(pause);
        digitalWrite(outputDownPin, RelayOn);
        delay(pause);
        allOff();

}
void allOff() {
        digitalWrite(outputUpPin, !RelayOn);
        digitalWrite(outputDownPin, !RelayOn);
        inputUp_lastState = digitalRead(inputUpPin);
        inputDown_lastState = digitalRead(inputDownPin);
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir, "up") == 0) {
                states[0] = RelayOn;
                states[1] = !RelayOn;
        }
        else if (strcmp(dir, "down") == 0) {
                states[0] = !RelayOn;
                states[1] = RelayOn;
        }
        else if (strcmp(dir, "off") == 0) {
                states[0] = !RelayOn;
                states[1] = !RelayOn;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (outputUp_currentState != states[0] && outputDown_currentState != states[1]) {
                allOff();
                delay(deBounceInt * 2);
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (outputUp_currentState != states[0] || outputDown_currentState != states[1]) {
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        if (iot.mqttConnected == true) {
                iot.pub_state(dir);
                sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
                iot.pub_msg(mqttmsg);
        }
}
void detectResetPresses() {
        if (millis() - lastResetPress < timeInterval_resetPress) {
                // if (useSerial) {
                //         Serial.print("Time between Press: ");
                //         Serial.println(millis() - lastResetPress);
                // }
                if (manResetCounter >= pressAmount2Reset) {
                        iot.sendReset("Manual operation");
                        // if (useSerial) {
                        //         Serial.println("Manual Reset initiated");
                        // }
                        manResetCounter = 0;
                }
                else {
                        manResetCounter++;
                }
        }
        else {
                manResetCounter = 0;
        }
}
void checkSwitch_PressedUp() {
        bool temp_inputUpPin = digitalRead(inputUpPin);

        if (temp_inputUpPin != inputUp_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputUpPin) != inputUp_lastState) {
                        if (digitalRead(inputUpPin) == SwitchOn) {
                                switchIt("Button", "up");
                                inputUp_lastState = digitalRead(inputUpPin);

                                detectResetPresses();
                                lastResetPress = millis();
                        }
                        else if (digitalRead(inputUpPin) == !SwitchOn) {
                                switchIt("Button", "off");
                                inputUp_lastState = digitalRead(inputUpPin);
                        }
                }

                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "UP Bounce: cRead [%d] lRead[%d]", temp_inputUpPin, inputUp_lastState);
                        iot.pub_msg(tMsg);
                }
        }

}
void checkSwitch_PressedDown() {
        bool temp_inputDownPin = digitalRead(inputDownPin);

        if (temp_inputDownPin != inputDown_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputDownPin) != inputDown_lastState) {

                        if (digitalRead(inputDownPin) == SwitchOn) {
                                switchIt("Button", "down");
                                inputDown_lastState = digitalRead(inputDownPin);
                        }
                        else if (digitalRead(inputDownPin) == !SwitchOn) {
                                switchIt("Button", "off");
                                inputDown_lastState = digitalRead(inputDownPin);
                        }
                }
                else { // for debug only
                        char tMsg [100];
                        sprintf(tMsg, "Down Bounce: cRead[%d] lRead[%d]", temp_inputDownPin, inputDown_lastState);
                        iot.pub_msg(tMsg);
                }
        }
}
void verifyNotHazardState() {
        if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn ) {
                switchIt("Button", "off");
                // if (useSerial) {
                //         Serial.println("Hazard state - both switches were ON");
                // }
                iot.sendReset("HazradState");
        }

}
void readGpioStates() {
        outputUp_currentState = digitalRead(outputUpPin);
        outputDown_currentState = digitalRead(outputDownPin);
        inputDown_currentState = digitalRead(inputDownPin);
        inputUp_currentState = digitalRead(inputUpPin);
}
void addiotnalMQTT(char incoming_msg[50]){
  char state[5];
  char state2[5];
  char msg[100];
  char msg2[100];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn) {
                        sprintf(state, "invalid Relay State");
                }
                else if (outputUp_currentState == !RelayOn && outputDown_currentState == RelayOn) {
                        sprintf(state, "DOWN");
                }
                else if (outputUp_currentState == RelayOn && outputDown_currentState == !RelayOn) {
                        sprintf(state, "UP");
                }
                else {
                        sprintf(state, "OFF");
                }

                // switch state
                if (inputUp_lastState == !RelayOn && inputDown_lastState == !RelayOn) {
                        sprintf(state2, "OFF");
                }
                else if (inputUp_lastState == RelayOn && inputDown_lastState == !RelayOn) {
                        sprintf(state2, "UP");
                }
                else if (inputUp_lastState == !RelayOn && inputDown_lastState == RelayOn) {
                        sprintf(state2, "DOWN");
                }
                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "up") == 0 || strcmp(incoming_msg, "down") == 0 || strcmp(incoming_msg, "off") == 0) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: Up[%d] Down[%d], Relay: Up[%d] Down[%d]", inputUpPin, inputDownPin, outputUpPin, outputDownPin);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                iot.pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s]", VER,iot.ver);
                iot.pub_msg(msg);
        }
}
void loop() {
        iot.looper(); // check wifi, mqtt, wdt

        readGpioStates();
        verifyNotHazardState(); // both up and down are ---> OFF

        checkSwitch_PressedUp();
        checkSwitch_PressedDown();

        delay(50);
}
