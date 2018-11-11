#include <Arduino.h>

#define RelayOn HIGH
#define SwitchOn LOW

#define input_1_UpPin 2    // local switches
#define input_1_DownPin 3
#define input_2_UpPin 4    // remote switches
#define input_2_DownPin 5
#define output_1_UpPin  6  // local relays
#define output_1_DownPin 7
#define output_2_UpPin 8   // remote to esp
#define output_2_DownPin 9

// rel_status set
bool input_1_UpPin_lastState = !SwitchOn;
bool input_1_DownPin_lastState= !SwitchOn;
bool input_2_UpPin_lastState= !SwitchOn;
bool input_2_DownPin_lastState= !SwitchOn;
bool input_1_UpPin_curState= !SwitchOn;
bool input_1_DownPin_curState= !SwitchOn;
bool input_2_UpPin_curState= !SwitchOn;
bool input_2_DownPin_curState= !SwitchOn;

// reset timing parameters
unsigned long lastPressTime = 0;
unsigned long lastTiming = 0;
int resetCounter = 0;
const int pressTimeInterval = 2000;
const int pressAmount = 10;
int debounceInt = 100; // milliseconds debounce def

void setup() {
        Serial.begin(9600);
        Serial.println("Arduino Master- Begins");

        startGPIOs();
        allOff();
}

void startGPIOs() {
        // INPUTS
        pinMode(input_1_UpPin, INPUT_PULLUP);
        pinMode(input_1_DownPin, INPUT_PULLUP);
        pinMode(input_2_UpPin, INPUT_PULLUP);
        pinMode(input_2_DownPin, INPUT_PULLUP);

        // OUTPUTS
        pinMode(output_1_UpPin, OUTPUT);
        pinMode(output_1_DownPin, OUTPUT);
        pinMode(output_2_UpPin, OUTPUT);
        pinMode(output_2_DownPin, OUTPUT);

}

void allOff() {
        digitalWrite(output_1_UpPin, !RelayOn);
        digitalWrite(output_1_DownPin, !RelayOn);
        digitalWrite(output_2_UpPin, !RelayOn);
        digitalWrite(output_2_DownPin, !RelayOn);

}

void sendReset() {
        //  digitalWrite(resetPin, HIGH);
        //  delay(500);
        //  digitalWrite(resetPin, LOW);
        Serial.println("RESET CMD SENT");
}

void read_inputCurrentState() {
        input_1_DownPin_curState = digitalRead(input_1_DownPin);
        input_1_UpPin_curState = digitalRead(input_1_UpPin);
        input_2_UpPin_curState = digitalRead(input_2_UpPin);
        input_2_DownPin_curState = digitalRead(input_2_DownPin);
}

void check_input_1_pressedUp() {
        if (input_1_UpPin_curState != input_1_UpPin_lastState) {
                delay(debounceInt);
                if (digitalRead(input_1_UpPin) != input_1_UpPin_lastState) {
                        if (digitalRead(output_1_DownPin) == RelayOn) {
                                digitalWrite(output_1_DownPin, !RelayOn);
                                delay(debounceInt);
                        }
                        digitalWrite(output_1_UpPin, !input_1_UpPin_curState);
                        input_1_UpPin_lastState = digitalRead(input_1_UpPin);

                        //reset by user
                        detectResetPresses();
                        lastPressTime = millis();

                        Serial.print("UpLocal:");
                        Serial.println(digitalRead(input_1_UpPin));
                }
        }
}

void check_input_1_pressedDown() {
        if (input_1_DownPin_curState != input_1_DownPin_lastState) {
                delay(debounceInt);
                if (digitalRead(input_1_DownPin) != input_1_DownPin_lastState ) {
                        if (digitalRead(output_1_UpPin) == RelayOn ) {
                                digitalWrite(output_1_UpPin, !RelayOn);
                                delay(debounceInt);
                        }

                        digitalWrite(output_1_DownPin, !input_1_DownPin_curState);
                        input_1_DownPin_lastState = digitalRead(input_1_DownPin);

                        Serial.print("DownLocal:");
                        Serial.println(digitalRead(input_1_DownPin));
                }
        }
}

void check_input_2_pressedUp() {
        if (input_2_UpPin_curState != input_2_UpPin_lastState) {
                delay(debounceInt);
                if (digitalRead(input_2_UpPin) != input_2_UpPin_lastState) {//} && output_1_UpPin != RelayOn) {
                        if (digitalRead(output_1_DownPin) == RelayOn ){//}&& input_2_UpPin_curState == SwitchOn) {
                                digitalWrite(output_1_DownPin, !RelayOn);
                                delay(debounceInt);
                        }

                        digitalWrite(output_1_UpPin, !input_2_UpPin_curState);
                        input_2_UpPin_lastState = digitalRead(input_2_UpPin);

                        // //ON OR OFF
                        // if (digitalRead(input_2_UpPin) == SwitchOn ) {
                        //         digitalWrite(output_1_UpPin, RelayOn);
                        // }
                        // else if (digitalRead(input_2_UpPin) == !SwitchOn ) {
                        //         digitalWrite(output_1_UpPin, !RelayOn);
                        // }
                        // input_2_UpPin_lastState = digitalRead(input_2_UpPin);


                        Serial.print("UpRemote:");
                        Serial.println(digitalRead(input_2_UpPin));
                }
        }

}

void check_input_2_pressedDown() {
        if (input_2_DownPin_curState != input_2_DownPin_lastState) {
                delay(debounceInt);
                if (digitalRead(input_2_DownPin) != input_2_DownPin_lastState ){//&& output_1_DownPin != RelayOn) {
                        if (digitalRead(output_1_UpPin) == RelayOn) {
                                digitalWrite(output_1_UpPin, !RelayOn);
                        }
                        digitalWrite(output_1_DownPin, !input_2_DownPin_curState);
                        input_2_DownPin_lastState = digitalRead(input_2_DownPin);

                        // //ON OR OFF
                        // if (digitalRead(input_2_DownPin) == SwitchOn ) {
                        //         digitalWrite(output_1_DownPin, RelayOn);
                        // }
                        // else if (digitalRead(input_2_DownPin) == !SwitchOn ) {
                        //         digitalWrite(output_1_DownPin, !RelayOn);
                        // }
                        // input_2_DownPin_lastState = digitalRead(input_2_DownPin);

                        Serial.print("DownRemote:");
                        Serial.println(digitalRead(input_2_DownPin));
                }
        }

}

void detectResetPresses() {
        // reset prcodure of remote board
        if (millis() - lastPressTime <= pressTimeInterval) {
                resetCounter = resetCounter + 1;
                if (resetCounter >= pressAmount) {
                        sendReset();
                        resetCounter = 0;
                }
        }
        else {
                resetCounter = 0;
        }
}

void loop() {

        read_inputCurrentState();
        //update output status pins
        digitalWrite(output_2_UpPin, digitalRead(output_1_UpPin));
        digitalWrite(output_2_DownPin, digitalRead(output_1_DownPin));

        // Local inputs
        check_input_1_pressedUp();
        check_input_1_pressedDown();

        //  Remote Inputs
        check_input_2_pressedUp();
        check_input_2_pressedDown();

        delay(50);
}
