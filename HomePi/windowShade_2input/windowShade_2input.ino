#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Windows/SaloonExit"
//must be defined to use myIOT
#define ADD_MQTT_FUNC addiotnalMQTT
//~~~
#define USE_SERIAL false
#define USE_WDT true
#define USE_OTA true
#define USE_MAN_RESET true
#define USE_BOUNCE_DEBUG false

#define VER "NodeMCU_2.1"
//####################################################
// device state definitions
#define RelayOn LOW
#define SwitchOn LOW

const int deBounceInt = 50;

// manual RESET parameters
int manResetCounter = 0;  // reset press counter
int pressAmount2Reset = 3; // time to press button to init Reset
long lastResetPress = 0; // time stamp of last press
const int timeInterval_resetPress = 1500;
// ####################


// GPIO Pins for ESP8266
//~~~~Internal Switch ~~~~~~
const int inputUpPin = 4;
const int inputDownPin = 5;
const int outputUpPin = 14;
const int outputDownPin = 12;
//~~~~External Input ~~~~~~~
const int inputUpExtPin = 0;
const int inputDownExtPin = 2;
//##########################

// GPIO status flags
//~~~~Internal Switch ~~~~~~
bool outputUp_currentState;
bool outputDown_currentState;
bool inputUp_lastState;
bool inputDown_lastState;
bool inputUp_currentState;
bool inputDown_currentState;
//~~~~External Input ~~~~~~~
bool inputUpExt_lastState;
bool inputDownExt_lastState;
bool inputUpExt_currentState;
bool inputDownExt_currentState;
// ###########################

myIOT iot(DEVICE_TOPIC);

void setup() {
        startGPIOs();
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker
}
void startGPIOs() {
        pinMode(inputUpPin, INPUT_PULLUP);
        pinMode(inputDownPin, INPUT_PULLUP);
        pinMode(inputUpExtPin, INPUT_PULLUP);
        pinMode(inputDownExtPin, INPUT_PULLUP);
        pinMode(outputUpPin, OUTPUT);
        pinMode(outputDownPin, OUTPUT);

        allOff();
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
        else if (strcmp(incoming_msg, "pbit") == 0 ) {
                iot.pub_msg("PowerOnBit");
                PBit();
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: Up_int[%d], Up_ext[%d], Down_int[%d], Down_ext[%d], Relay: Up[%d] Down[%d]", inputUpPin, inputUpExtPin, inputDownPin, outputUpPin, inputDownExtPin, outputDownPin);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], MAN_RESET:[%d]", VER,iot.ver, USE_WDT, USE_OTA, USE_SERIAL,USE_MAN_RESET);
                iot.pub_msg(msg);
        }
}

// ~~~~ maintability ~~~~~~
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
        inputUpExt_lastState = digitalRead(inputUpExtPin);
        inputDownExt_lastState = digitalRead(inputDownExtPin);
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
                if (manResetCounter >= pressAmount2Reset) {
                        iot.sendReset("Manual operation");
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
        if (digitalRead(inputUpPin) != inputUp_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputUpPin) != inputUp_lastState) {
                        if (digitalRead(inputUpPin) == SwitchOn) {
                                switchIt("inButton", "up");
                                inputUp_lastState = digitalRead(inputUpPin);
                                if (USE_MAN_RESET) {
                                        detectResetPresses();
                                        lastResetPress = millis();
                                }
                        }
                        else if (digitalRead(inputUpPin) == !SwitchOn) {
                                switchIt("inButton", "off");
                                inputUp_lastState = digitalRead(inputUpPin);
                        }
                }
                else { // for debug only
                        if (USE_BOUNCE_DEBUG) {
                                char tMsg [100];
                                sprintf(tMsg, "UP Bounce: cRead [%d] lRead[%d]", digitalRead(inputUpPin), inputUp_lastState);
                                iot.pub_msg(tMsg);
                        }
                }
        }
}
void checkSwitch_PressedUpExt() {
        if (digitalRead(inputUpExtPin) != inputUpExt_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputUpExtPin) != inputUpExt_lastState) {
                        if (digitalRead(inputUpExtPin) == SwitchOn) {
                                switchIt("extButton", "up");
                                inputUpExt_lastState = digitalRead(inputUpExtPin);
                        }
                        else if (digitalRead(inputUpExtPin) == !SwitchOn) {
                                switchIt("extButton", "off");
                                inputUpExt_lastState = digitalRead(inputUpExtPin);
                        }
                }

                else {
                        if(USE_BOUNCE_DEBUG) {
                                char tMsg [100];
                                sprintf(tMsg, "extUP Bounce: cRead [%d] lRead[%d]", digitalRead(inputUpExtPin), inputUpExt_lastState);
                                iot.pub_msg(tMsg);
                        }
                }
        }

}
void checkSwitch_PressedDown() {
        if (digitalRead(inputDownPin) != inputDown_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputDownPin) != inputDown_lastState) {

                        if (digitalRead(inputDownPin) == SwitchOn) {
                                switchIt("inButton", "down");
                                inputDown_lastState = digitalRead(inputDownPin);
                        }
                        else if (digitalRead(inputDownPin) == !SwitchOn) {
                                switchIt("inButton", "off");
                                inputDown_lastState = digitalRead(inputDownPin);
                        }
                }
                else { // for debug only
                        if (USE_BOUNCE_DEBUG) {
                                char tMsg [100];
                                sprintf(tMsg, "Down Bounce: cRead[%d] lRead[%d]", digitalRead(inputDownPin), inputDown_lastState);
                                iot.pub_msg(tMsg);
                        }
                }
        }
}
void checkSwitch_PressedDownExt() {
        if ( digitalRead(inputDownExtPin) != inputDownExt_lastState) {
                delay(deBounceInt);
                if (digitalRead(inputDownExtPin) != inputDownExt_lastState) {
                        if (digitalRead(inputDownExtPin) == SwitchOn) {
                                switchIt("extButton", "down");
                                inputDownExt_lastState = digitalRead(inputDownExtPin);
                        }
                        else if (digitalRead(inputDownExtPin) == !SwitchOn) {
                                switchIt("extButton", "off");
                                inputDownExt_lastState = digitalRead(inputDownExtPin);
                        }
                }
                else { // for debug only
                        if(USE_BOUNCE_DEBUG) {
                                char tMsg [100];
                                sprintf(tMsg, "extDown Bounce: cRead[%d] lRead[%d]",  digitalRead(inputDownExtPin), inputDownExt_lastState);
                                iot.pub_msg(tMsg);
                        }
                }
        }
}
void verifyNotHazardState() {
        if (outputUp_currentState == RelayOn && outputDown_currentState == RelayOn ) {
                switchIt("Button", "off");
                iot.sendReset("HazradState");
        }

}
void readGpioStates() {
        outputUp_currentState = digitalRead(outputUpPin);
        outputDown_currentState = digitalRead(outputDownPin);
        inputDown_currentState = digitalRead(inputDownPin);
        inputUp_currentState = digitalRead(inputUpPin);
}

void loop() {
        iot.looper();
        readGpioStates();
        verifyNotHazardState(); // both up and down are ---> OFF

        // react to commands (MQTT or local switch)
        checkSwitch_PressedUp();
        checkSwitch_PressedDown();
        checkSwitch_PressedUpExt();
        checkSwitch_PressedDownExt();

        delay(50);
}
