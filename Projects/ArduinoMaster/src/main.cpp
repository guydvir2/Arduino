#include <Arduino.h>

// Pin defs
const int localInput_0 = 2;
const int localInput_1 = 3;
const int remoteInput_0 = 4;
const int remoteInput_1 = 5;
const int relay_0_Pin = 6;
const int relay_1_Pin = 7;
const int statusRel_0 = 8;
const int statusRel_1 = 9;
const int ledPin = 13;
const int resetPin = 10;

// rel_status set
bool lastState_localInput_0 = LOW;
bool lastState_localInput_1= LOW;
bool lastState_remoteInput_0= LOW;
bool lastState_remoteInput_1= LOW;

// reset timing parameters
unsigned int lastPressTime=0;
unsigned int lastTiming=0;
int resetCounter=0;
const int pressTimeInterval=2000;
const int pressAmount=10;

void setup() {
        Serial.begin(9600);
        Serial.println("HI GUY");
        // INPUTS
        pinMode(localInput_0, INPUT_PULLUP);
        pinMode(localInput_1, INPUT_PULLUP);
        pinMode(remoteInput_0, INPUT_PULLUP);
        pinMode(remoteInput_1, INPUT_PULLUP);
        // OUTPUTS
        pinMode(relay_0_Pin, OUTPUT);
        pinMode(relay_1_Pin, OUTPUT);
        pinMode(statusRel_0, OUTPUT);
        pinMode(statusRel_1, OUTPUT);
        pinMode(ledPin,OUTPUT);
        pinMode(resetPin, OUTPUT);
        // Status to EXT pin ( back to ESP port)
        digitalWrite(relay_0_Pin, LOW);
        digitalWrite(relay_1_Pin, LOW);
        digitalWrite(statusRel_0, LOW);
        digitalWrite(statusRel_1, LOW);
        digitalWrite(resetPin, LOW);
}
void sendReset(){
        digitalWrite(resetPin,HIGH);
        delay(500);
        digitalWrite(resetPin, LOW);
        Serial.println("RESET CMD SENT");
}
void loop() {
        // int lastTiming = millis();
        // read HW into local variables
        int read_localInput_0 = digitalRead(localInput_0);
        int read_localInput_1 = digitalRead(localInput_1);
        int read_remoteInput_0 = digitalRead(remoteInput_0);
        int read_remoteInput_1 = digitalRead(remoteInput_1);

        //update output status pins
        digitalWrite(statusRel_0, digitalRead(relay_0_Pin));
        digitalWrite(statusRel_1, digitalRead(relay_1_Pin));

        // Local inputs
        if (read_localInput_0!=lastState_localInput_0) {
                delay(50);
                if (read_localInput_0!=lastState_localInput_0) {
                        digitalWrite(relay_0_Pin, !read_localInput_0);
                        lastState_localInput_0=read_localInput_0;
                        Serial.println("localInput_0 detected");
                        // Send RESET request to Remote pins
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
        if (read_localInput_1!=lastState_localInput_1) {
                delay(50);
                if (read_localInput_1!=lastState_localInput_1) {
                        digitalWrite(relay_1_Pin, !read_localInput_1);
                        lastState_localInput_1=read_localInput_1;
                        Serial.println("in1");
                }
        }
        // ESP inputs
        if (read_remoteInput_0!=lastState_remoteInput_0) {
                delay(50);
                if (read_remoteInput_0!=lastState_remoteInput_0) {
                        digitalWrite(relay_0_Pin, !read_remoteInput_0);
                        lastState_remoteInput_0=read_remoteInput_0;
                        Serial.println("rem0");
                }
        }
        if (read_remoteInput_1!=lastState_remoteInput_1) {
                delay(50);
                if (read_remoteInput_1!=lastState_remoteInput_1) {
                        digitalWrite(relay_1_Pin, !read_remoteInput_1);
                        lastState_remoteInput_1=read_remoteInput_1;
                        Serial.println("rem1");

                }
        }

        delay(100);
}
