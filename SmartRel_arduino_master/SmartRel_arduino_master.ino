#include <Arduino.h>

// Pin defs
// local switches
const int switchUpLocal = 2;
const int switchDownLocal = 3;
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
//const int ledPin = 13;
const int resetPin = 10;
// ######################################

// rel_status set
bool switchUpLocal_lastState = 1;
bool switchDownLocal_lastState= 1;
bool switchUpRemote_lastState= 1;
bool switchDownRemote_lastState= 1;
bool switchUpLocal_curState = 1;
bool switchDownLocal_curState= 1;
bool switchUpRemote_curState= 1;
bool switchDownRemote_curState= 1;

// reset timing parameters
unsigned int lastPressTime=0;
unsigned int lastTiming=0;
int resetCounter=0;
const int pressTimeInterval=2000;
const int pressAmount=10;
int debounceInt=50; // milliseconds debounce def

void setup() {
  
        Serial.begin(9600);
        Serial.println("Arduino Master- Begins");
        // INPUTS
        pinMode(switchUpLocal, INPUT_PULLUP);
        pinMode(switchDownLocal, INPUT_PULLUP);
        pinMode(switchUpRemote, INPUT_PULLUP);
        pinMode(switchDownRemote, INPUT_PULLUP);
        // OUTPUTS
        pinMode(relayUpPin, OUTPUT);
        pinMode(relayDownPin, OUTPUT);
        pinMode(relayUp_statusPin, OUTPUT);
        pinMode(relayDown_statusPin, OUTPUT);
//        pinMode(ledPin,OUTPUT);
        pinMode(resetPin, OUTPUT);

        // Status to EXT pin ( back to ESP port)
        digitalWrite(relayUpPin, LOW);
        digitalWrite(relayDownPin, LOW);
        digitalWrite(relayUp_statusPin, LOW);
        digitalWrite(relayDown_statusPin, LOW);
//        digitalWrite(resetPin, LOW);
}

void sendReset(){
        digitalWrite(resetPin,HIGH);
        delay(500);
        digitalWrite(resetPin, LOW);
        Serial.println("RESET CMD SENT");
}

void readCurrentState(){
        switchDownLocal_curState= digitalRead(switchDownLocal);
        switchUpLocal_curState = digitalRead(switchUpLocal);
        switchUpRemote_curState = digitalRead(switchUpRemote);
        switchDownRemote_curState = digitalRead(switchDownRemote);
}

void checkSwitch_pressedUp(){
        if (switchUpLocal_curState!=switchUpLocal_lastState) {
                delay(debounceInt);
                if (digitalRead(switchUpLocal)!=switchUpLocal_lastState) {
                        if (digitalRead(relayDownPin)==HIGH) {
                                digitalWrite(relayDownPin,LOW);
                                switchDownLocal_lastState=switchDownLocal_curState;
                        }
//                      ON OR OFF
                        digitalWrite(relayUpPin, !switchUpLocal_curState);
                        switchUpLocal_lastState=switchUpLocal_curState;
                        Serial.print("UpLocal:");
                        Serial.println(digitalRead(switchUpLocal));

                        detectResetPresses();
                        Serial.print("ResetCounter: ");
                        Serial.println(resetCounter);
                }
        }

}

void checkSwitch_pressedDown(){
        if (switchDownLocal_curState!=switchDownLocal_lastState) {
                delay(debounceInt);
                if (digitalRead(switchDownLocal)!=switchDownLocal_lastState) {
                        if (digitalRead(relayUpPin)==HIGH) {
                                digitalWrite(relayUpPin,LOW);
                                switchUpLocal_lastState=switchUpLocal_curState;
                        }
                        digitalWrite(relayDownPin, !switchDownLocal_curState);
                        switchDownLocal_lastState=switchDownLocal_curState;
                        Serial.print("DownLocal:");
                        Serial.println(digitalRead(switchDownLocal));
                }
        }

}

void checkRemote_CmdUp(){
//        if (switchUpRemote_curState!=switchUpRemote_lastState) {
//                delay(debounceInt);
//                if (digitalRead(switchUpRemote)!=switchUpRemote_lastState) {
//                        if (digitalRead(relayDownPin)==HIGH) {
//                                digitalWrite(relayDownPin,LOW);
//                                switchDownRemote_lastState=switchDownRemote_curState;
//                        }
//                        digitalWrite(relayUpPin, !switchUpRemote_curState);
//                        switchUpRemote_lastState=switchUpRemote_curState;
//                        Serial.println("UpRemote");
//                        Serial.println(digitalRead(switchUpRemote));
//                }
        Serial.println("RempteUp is pressed");
        }

void checkRemote_CmdDown(){
//        if (switchDownRemote_curState!=switchDownRemote_lastState) {
//                delay(debounceInt);
//                if (digitalRead(switchDownRemote)!=switchDownRemote_lastState) {
//                        if (digitalRead(relayUpPin)==HIGH) {
//                                digitalWrite(relayUpPin,LOW);
//                                switchUpRemote_lastState=switchUpRemote_curState;
//                        }
//                        digitalWrite(relayDownPin, !switchDownRemote_curState);
//                        switchDownRemote_lastState=switchDownRemote_curState;
//                        Serial.print("DownRemote:");
//                        Serial.println(digitalRead(switchDownRemote));
//                }
//        }
Serial.prinln("Remote Down");
}

void detectResetPresses(){
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

}

void switchIt(char *dir){
        char mqttmsg[50];
        bool states[2];
        // system states: up, down, off
        if (strcmp(dir,"down")==0) {
                states[0]=LOW;
                states[1]=HIGH;
        }
        else if (strcmp(dir,"up")==0) {
                states[0]=HIGH;
                states[1]=LOW;
        }
        else if (strcmp(dir,"off")==0) {
                states[0]=LOW;
                states[1]=LOW;
        }

        // Case that both realys need to change state ( Up --> Down or Down --> Up )
        if (digitalRead(relayUpPin) != states[0] && digitalRead(relayDownPin) != states[1]) {
                digitalWrite(relayUpPin, LOW);
                digitalWrite(relayDownPin, LOW);
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

void gpio_SWstatus(){
        Serial.print("up:");
        Serial.println(digitalRead(switchUpLocal));
        Serial.print("down:");
        Serial.println(digitalRead(switchDownLocal));
}

void gpio_RELstatus(){
        Serial.print("up:");
        Serial.println(digitalRead(relayUpPin));
        Serial.print("down:");
        Serial.println(digitalRead(relayDownPin));
}

void loop() {
        readCurrentState();

        //update output status pins
        digitalWrite(relayUp_statusPin, digitalRead(relayUpPin));
        digitalWrite(relayDown_statusPin, digitalRead(relayDownPin));

        // Local inputs
        checkSwitch_pressedUp();
        checkSwitch_pressedDown();

//        gpio_RELstatus();

        // ESP inputs
        checkRemote_CmdUp();
        checkRemote_CmdDown();
//          gpio_SWstatus();

        delay(50);
}
