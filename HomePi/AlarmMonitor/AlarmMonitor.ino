/*
ESP8266 Alarm monitoring system, by guyDvir Apr.2019

Pupose of this code is to monitor, notify and change syste, states of an existing
Alarm system ( currently using PIMA alarm system incl. RF detectors) has the
following hardware:
1) Main Controller AKA " the brain"
2) KeyPad for entering user codes and change syste states.
3) "the brain" has relay inputs knows as external key operation:
 */


#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/alarmMonitor"
#define USE_SERIAL   false
#define USE_WDT      true
#define USE_OTA      true
#define VER          "NodeMCU_2.1"
// ###################################################


// device state definitions
#define RelayOn HIGH
#define SwitchOn LOW


// GPIO Pins for ESP8266
const int systemState_armed_Pin = 4;  // input (System State)
const int systemState_alarm_Pin = 5;  // input (System State)
const int armedHomePin          = 12; // Output (Set system)
const int armedAwayPin          = 14; // Output (Set system)
//##########################

const int systemPause = 2000; // seconds, delay to system react
const int deBounceInt = 50;

// GPIO status flags
bool armedHome_currentState;
bool armedAway_currentState;
bool systemState_armed_lastState;
bool systemState_alarm_lastState;
bool systemState_armed_currentState;
bool systemState_alarm_currentState;
// ################################

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);

void setup() {
        startGPIOs();
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker
}
void startGPIOs() {
        pinMode(systemState_armed_Pin, INPUT_PULLUP);
        pinMode(systemState_alarm_Pin, INPUT_PULLUP);
        pinMode(armedHomePin, OUTPUT);
        pinMode(armedAwayPin, OUTPUT);

        systemState_alarm_currentState = digitalRead(systemState_alarm_Pin);
        systemState_armed_currentState = digitalRead(systemState_armed_Pin);
}
void addiotnalMQTT(char incoming_msg[50]){
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                if (digitalRead(armedHomePin) == RelayOn && digitalRead(armedAwayPin) == RelayOn) {
                        sprintf(state, "Status: invalid [Armed] and [Away] State");
                }
                else if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn && digitalRead(systemState_armed_Pin) == SwitchOn) {
                        sprintf(state, "Status: Manual [Armed]");
                }
                else if (digitalRead(armedHomePin) == RelayOn && digitalRead(armedAwayPin) == !RelayOn && digitalRead(systemState_armed_Pin) == SwitchOn) {
                        sprintf(state, "Status: [Code] [Home Armed]");
                }
                else if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == RelayOn && digitalRead(systemState_armed_Pin) == SwitchOn) {
                        sprintf(state, "Status: [Code] [Armed Away]");
                }
                else if (digitalRead(systemState_armed_Pin) == SwitchOn && digitalRead(systemState_alarm_Pin)== SwitchOn) {
                        sprintf(state, "Status: [Alarm]");
                }
                else if (digitalRead(systemState_armed_Pin) == !SwitchOn && digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                        sprintf(state, "Status: [disarmed]");
                }
                else {
                        sprintf(state, "Status: [notDefined]");

                }

                iot.pub_msg(state);
        }
        else if (strcmp(incoming_msg, "armed_home") == 0 || strcmp(incoming_msg, "armed_away") == 0 || strcmp(incoming_msg, "disarmed") == 0) {
                switchIt("MQTT", incoming_msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d]", VER,iot.ver, USE_WDT, USE_OTA, USE_SERIAL);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "pins") == 0 ) {
                sprintf(msg, "Switch: input1[%d] input2[%d], Relay: output_home[%d] output_full[%d]", systemState_armed_Pin, systemState_alarm_Pin, armedHomePin, armedAwayPin);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "clear") == 0 ) {
                allReset();
                iot.sendReset("clear");
        }
}

// ~~~~ maintability ~~~~~~
void allOff() {
        digitalWrite(armedHomePin, !RelayOn);
        digitalWrite(armedAwayPin, !RelayOn);
        delay(systemPause);
        readGpioStates();
}
void allReset() {
        digitalWrite(armedHomePin, RelayOn);
        delay(systemPause);
        digitalWrite(armedHomePin, !RelayOn);
        delay(systemPause);
        digitalWrite(armedAwayPin, RelayOn);
        delay(systemPause);
        digitalWrite(armedAwayPin, !RelayOn);
        delay(systemPause);

        readGpioStates();
}

// ~~~~~~~~~ GPIO switching ~~~~~~~~~~~~~
void switchIt(char *type, char *dir) {
        char mqttmsg[50];
        bool states[2];

        if (strcmp(dir, "armed_home") == 0 ) {
                if (digitalRead(armedHomePin) == !RelayOn) { // verify it is not in desired state already
                        if ( digitalRead(armedAwayPin) == RelayOn) { // in armed away state
                                digitalWrite(armedAwayPin, !RelayOn);
                                iot.pub_msg("[Code] [Disarmed Away]");
                                delay(systemPause);
                        }

                        digitalWrite(armedHomePin, RelayOn);         // Now switch to armed_home
                        delay(systemPause);

                        if (digitalRead(systemState_armed_Pin)==SwitchOn) {
                                iot.pub_msg("[Code] [Armed Home]");
                                iot.pub_state("armed_home");
                        }
                        else {
                                allOff();
                                iot.pub_msg("failed to [Armed Home]. [Code] [Disarmed]");
                        }
                }
                else {
                        iot.pub_msg("already in [Armed Home]");
                }
        }
        else if (strcmp(dir, "armed_away") == 0) {
                if ( digitalRead(armedAwayPin) == !RelayOn) {
                        if ( digitalRead(armedHomePin) == RelayOn) { // armed home
                                digitalWrite(armedHomePin, !RelayOn);
                                iot.pub_msg("[Code] [Disarmed Home]");
                                delay(systemPause);
                        }

                        digitalWrite(armedAwayPin, RelayOn); // now switch to Away
                        delay(systemPause);

                        if (digitalRead(systemState_armed_Pin)==SwitchOn) {
                                iot.pub_msg("[Code] [Armed Away]");
                                iot.pub_state("armed_away");
                        }
                        else {
                                allOff();
                                iot.pub_msg("failed to [Armed Away]. [Code] [Disarmed]");
                        }
                }
                else {
                        iot.pub_msg("already in [Armed Away]");
                }
        }
        else if (strcmp(dir, "disarmed") == 0) {
                if (systemState_armed_lastState == SwitchOn) { // system is armed
                        if (digitalRead(armedAwayPin)==RelayOn || digitalRead(armedHomePin)==RelayOn) { // case of Remote operation
                                allOff();
                        }
                        else { // case of manual operation
                                // initiate any arm state in order to disarm
                                digitalWrite(armedHomePin, RelayOn);
                                delay(systemPause);
                                allOff();
                        }
                        if (digitalRead(systemState_armed_Pin)==!SwitchOn && digitalRead(armedAwayPin)==!RelayOn && digitalRead(armedHomePin)==!RelayOn) {
                                iot.pub_msg("[Disarmed]");
                                iot.pub_state("disarmed");
                        }
                        else {
                                iot.pub_msg("error trying to [Disarm]");
                        }
                }
        }
}
void check_systemState_armed() { // System OUTPUT 1: arm_state
        if (digitalRead(systemState_armed_Pin) != systemState_armed_lastState) {
                delay(deBounceInt);
                if (digitalRead(systemState_armed_Pin) != systemState_armed_lastState) {
                        delay(systemPause);

                        if (digitalRead(systemState_armed_Pin) == SwitchOn) { // system is set to armed
                                if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                                        iot.pub_msg("detected: [Manual] [Armed]");
                                        iot.pub_state("pending");
                                }
                                else if (digitalRead(armedAwayPin) == RelayOn || digitalRead(armedHomePin) == RelayOn) {
                                        iot.pub_msg("detected: [Code] [Armed]");
                                }
                        }

                        else { // system Disarmed
                                if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                                        iot.pub_msg("detected: [Disarmed]");
                                        iot.pub_state("disarmed");
                                }
                                else {
                                        allOff();
                                        if (digitalRead(armedHomePin) == !RelayOn && digitalRead(armedAwayPin) == !RelayOn) {
                                                iot.pub_msg("detected: [Code] [Disarmed]");
                                                iot.pub_state("disarmed");
                                        }
                                        else {
                                                iot.pub_msg("failed to [Disarm]");
                                                iot.sendReset("failed to Disarm");
                                        }
                                }
                        }
                        systemState_armed_lastState = digitalRead(systemState_armed_Pin);
                }
        }
}
void check_systemState_alarming() { // // System OUTPUT 2: alarm_state
        if (digitalRead(systemState_alarm_Pin) != systemState_alarm_lastState) {
                delay(deBounceInt);
                if (digitalRead(systemState_alarm_Pin) != systemState_alarm_lastState) {
                        delay(systemPause);
                        // alarm set off
                        if (digitalRead(systemState_alarm_Pin) == SwitchOn) {
                                iot.pub_msg("System is [Alarming!]");
                                iot.pub_state("triggered");
                        }
                        // alarm ended
                        else if (digitalRead(systemState_alarm_Pin) == !SwitchOn) {
                                iot.pub_msg("System stopped [Alarming]");
                                if (digitalRead(systemState_armed_Pin)==SwitchOn) {
                                        if (digitalRead(armedAwayPin)==RelayOn) {
                                                iot.pub_state("armed_away");
                                        }
                                        else if (digitalRead(armedHomePin)==RelayOn) {
                                                iot.pub_state("armed_home");
                                        }
                                        else{
                                                iot.pub_state("pending");
                                        }
                                }
                                else{
                                        iot.pub_state("disarmed");
                                }
                        }
                        systemState_alarm_lastState = digitalRead(systemState_alarm_Pin);
                }
        }
}
void readGpioStates() {
        armedHome_currentState         = digitalRead(armedHomePin);
        armedAway_currentState         = digitalRead(armedAwayPin);
        systemState_alarm_currentState = digitalRead(systemState_alarm_Pin);
        systemState_armed_currentState = digitalRead(systemState_armed_Pin);
}

void loop() {
        iot.looper();
        readGpioStates();

        check_systemState_armed();
        check_systemState_alarming();

        delay(50);
}
