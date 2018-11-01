#include <Arduino.h>

// Pin defs
// local switches
const int switchUpLocal = 2;
const int switchDownpLocal = 3;
// remote switches by ESP, via MQTT commands
const int switchUpRemote = 4;
const int switchDownRemote = 5;
// Relays
const int relayUpPin = 6;
const int relayDownPin = 7;
// mirror status to other GPIO for ESP to read
const int relayUp_statusPin = 8;
const int relayDown_statusPin = 9;
// others
const int ledPin = 13;
const int resetPin = 10;
// ######################################

// rel_status set
bool switchUpLocal_lastState = LOW;
bool switchDownLocal_lastState= LOW;
bool switchUpRemote_lastState= LOW;
bool switchDownRemote_lastState= LOW;
bool switchUpLocal_curState = LOW;
bool switchDownLocal_curState= LOW;
bool switchUpRemote_curState= LOW;
bool switchDownRemote_curState= LOW;

// reset timing parameters
unsigned int lastPressTime=0;
unsigned int lastTiming=0;
int resetCounter=0;
const int pressTimeInterval=2000;
const int pressAmount=10;
int debounceInt=50; // milliseconds debounce def

void setup() {
        Serial.begin(9600);
        // INPUTS
        pinMode(switchUpLocal, INPUT_PULLUP);
        pinMode(switchDownpLocal, INPUT_PULLUP);
        pinMode(switchUpRemote, INPUT_PULLUP);
        pinMode(switchDownRemote, INPUT_PULLUP);
        // OUTPUTS
        pinMode(relayUpPin, OUTPUT);
        pinMode(relayDownPin, OUTPUT);
        pinMode(relayUp_statusPin, OUTPUT);
        pinMode(relayDown_statusPin, OUTPUT);
        pinMode(ledPin,OUTPUT);
        pinMode(resetPin, OUTPUT);
        // Status to EXT pin ( back to ESP port)
        digitalWrite(relayUpPin, LOW);
        digitalWrite(relayDownPin, LOW);
        digitalWrite(relayUp_statusPin, LOW);
        digitalWrite(relayDown_statusPin, LOW);
        digitalWrite(resetPin, LOW);
}

void sendReset(){
        digitalWrite(resetPin,HIGH);
        delay(500);
        digitalWrite(resetPin, LOW);
        Serial.println("RESET CMD SENT");
}

void readCurrentState(){
        switchDownLocal_curState= digitalRead(switchDownpLocal);
        switchUpLocal_curState = digitalRead(switchUpLocal);
        switchUpRemote_curState = digitalRead(switchUpRemote);
        switchDownRemote_curState = digitalRead(switchDownRemote);
}

void checkSwitch_pressedUp(){
        if (switchUpLocal_curState!=switchUpLocal_lastState) {
                delay(debounceInt);
                if (digitalRead(switchUpLocal)!=switchUpLocal_lastState) {
                        switchIt("up");
                        // digitalWrite(relayUpPin, !switchUpLocal_curState);
                        switchUpLocal_lastState=switchUpLocal_curState;
                        Serial.println("switchUpLocal detected");
                        // reset prcodure of remote board
                        if (millis()-lastPressTime <= pressTimeInterval) {
                                resetCounter=resetCounter+1;
                                if (resetCounter>=pressAmount) {
                                        sendReset();
                                        resetCounter=0;
                                }
                        }
                        else {
                                resetCounter = 0;
                        }
                        lastPressTime = millis();
                        Serial.println(resetCounter);
                }
        }

}

void checkSwitch_pressedDown(){
        if (switchDownLocal_curState!=switchDownLocal_lastState) {
                delay(debounceInt);
                if (digitalRead(switchDownpLocal)!=switchDownLocal_lastState) {
                        switchIt("down");
                        // digitalWrite(relayDownPin, !switchDownLocal_curState);
                        switchDownLocal_lastState=switchDownLocal_curState;
                        Serial.println("in1");
                }
        }

}

void checkRemote_CmdUp(){
        if (switchUpRemote_curState!=switchUpRemote_lastState) {
                delay(debounceInt);
                if (digitalRead(switchUpRemote)!=switchUpRemote_lastState) {
                        switchIt("up");
                        // digitalWrite(relayUpPin, !switchUpRemote_curState);
                        switchUpRemote_lastState=switchUpRemote_curState;
                        Serial.println("rem0");
                }
        }

}

void checkRemote_CmdDown(){
        if (switchDownRemote_curState!=switchDownRemote_lastState) {
                delay(debounceInt);
                if (digitalRead(switchDownRemote)!=switchDownRemote_lastState) {
                        switchIt("down");
                        // digitalWrite(relayDownPin, !switchDownRemote_curState);
                        switchDownRemote_lastState=switchDownRemote_curState;
                        Serial.println("rem1");
                }
        }

}

void switchIt(char *dir){
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir,"up")==0) {
                states[0]=LOW;
                states[1]=HIGH;
        }
        else if (strcmp(dir,"down")==0) {
                states[0]=HIGH;
                states[1]=LOW;
        }
        else if (strcmp(dir,"off")==0) {
                states[0]=HIGH;
                states[1]=HIGH;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (digitalRead(relayUpPin) != states[0] && digitalRead(relayDownPin) != states[1]) {
                digitalWrite(relayUpPin, HIGH);
                digitalWrite(relayDownPin, HIGH);
                delay(debounceInt);
                digitalWrite(relayUpPin, states[0]);
                digitalWrite(relayDownPin, states[1]);
        }
        // Case that one relay changes from/to off --> on
        else if (digitalRead(relayUpPin) != states[0] || digitalRead(relayDownPin) != states[1]) {
                digitalWrite(relayUpPin, states[0]);
                digitalWrite(relayDownPin, states[1]);
        }
//         // updates relay states
//         relayUp_currentState=states[0];
//         relayDown_currentState=states[1];
}

void loop() {
        readCurrentState();
        
        //update output status pins
        digitalWrite(relayUp_statusPin, digitalRead(relayUpPin));
        digitalWrite(relayDown_statusPin, digitalRead(relayDownPin));
        
        // Local inputs
        checkSwitch_pressedUp();
        checkSwitch_pressedDown();
        
        // ESP inputs
        checkRemote_CmdUp();
        checkRemote_CmdDown();

        delay(100);
}

