#include <myIOT.h>
#include <Arduino.h>

//############################################K########
#define DEVICE_TOPIC "HomePi/Dvir/Windows/SaloonDual"

#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_BOUNCE_DEBUG false

#define VER "NodeMCU_4.0"
//####################################################

// state definitions
#define RelayOn  LOW
#define SwitchOn LOW

// GPIO Pins for ESP8266
const int inputUpPin    = 4;
const int inputDownPin  = 5;
const int outputUpPin   = 14;
const int outputDownPin = 12;
//##########################

// GPIO status flags
bool inputUp_lastState;
bool inputDown_lastState;
volatile bool up_interruptState   = false;
volatile bool down_interruptState = false;
// ###########################

const int deBounceInt = 50;


#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);

void setup() {
        startGPIOs();
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);
}

// ~~~~~~~~~ StartUp ~~~~~~~~~~~~
void startGPIOs() {
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);

        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        attachInterrupt(digitalPinToInterrupt(inputUpPin), up_handleInterrupt, CHANGE);
        attachInterrupt(digitalPinToInterrupt(inputDownPin), down_handleInterrupt, CHANGE);

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
        if (digitalRead(outputUpPin) != states[0] && digitalRead(outputDownPin) != states[1]) {
                allOff();
                delay(deBounceInt * 2);
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (digitalRead(outputUpPin) != states[0] || digitalRead(outputDownPin) != states[1]) {
                digitalWrite(outputUpPin, states[0]);
                digitalWrite(outputDownPin, states[1]);
        }

        // delay(100);

        if (iot.mqttConnected == true) {
                iot.pub_state(dir);
                sprintf(mqttmsg, "[%s] switched [%s]", type, dir);
                iot.pub_msg(mqttmsg);
        }
}
void upPressed() {
        if (digitalRead(inputUpPin) != inputUp_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputUpPin) != inputUp_lastState) {
                        if (digitalRead(inputUpPin) == SwitchOn) {
                                switchIt("Button", "up");
                                inputUp_lastState = digitalRead(inputUpPin);
                        }
                        else if (digitalRead(inputUpPin) == !SwitchOn) {
                                switchIt("Button", "off");
                                inputUp_lastState = digitalRead(inputUpPin);
                        }
                }
        }
}
void downPressed() {
        if (digitalRead(inputDownPin) != inputDown_lastState) {
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
        }
}
void verifyNotHazardState() {
        if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn ) {
                switchIt("Button", "off");
                iot.sendReset("HazradState");
        }

}
void addiotnalMQTT(char incoming_msg[50]) {
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == RelayOn) {
                        sprintf(state, "invalid Relay State");
                }
                else if (digitalRead(outputUpPin) == !RelayOn && digitalRead(outputDownPin) == RelayOn) {
                        sprintf(state, "DOWN");
                }
                else if (digitalRead(outputUpPin) == RelayOn && digitalRead(outputDownPin) == !RelayOn) {
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
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL);
                iot.pub_msg(msg);
        }
}

void up_handleInterrupt() {
        detachInterrupt(digitalPinToInterrupt(inputUpPin));
        up_interruptState = true;
}
void down_handleInterrupt() {
        detachInterrupt(digitalPinToInterrupt(inputDownPin));
        down_interruptState = true;
}
void loop() {
        iot.looper();
        verifyNotHazardState();    // case both up and down are ---> OFF

        if (up_interruptState) {
                upPressed();
                up_interruptState = false;
                attachInterrupt(digitalPinToInterrupt(inputUpPin), up_handleInterrupt, CHANGE);
        }
        if (down_interruptState) {
                downPressed();
                down_interruptState = false;
                attachInterrupt(digitalPinToInterrupt(inputDownPin), down_handleInterrupt, CHANGE);
        }
        delay(100);
}
