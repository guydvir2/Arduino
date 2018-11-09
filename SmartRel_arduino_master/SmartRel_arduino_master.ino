#include <Arduino.h>

// Pin defs
// input_1: local switches
const int input_1_UpPin = 2;
const int input_1_DownPin = 3;
// input_2: remote switches by ESP, via MQTT commands
const int input_2_UpPin = 4;
const int input_2_DownPin = 5;
// Relays
const int output_1_UpPin = 6;
const int output_1_DownPin = 7;
// mirror status to other GPIO for ESP to read
const int output_2_UpPin = 8;
const int output_2_DownPin = 9;

// others
//const int resetPin = 10;
// ######################################

// rel_status set
bool input_1_UpPin_lastState = 1;
bool input_1_DownPin_lastState = 1;
bool input_2_UpPin_lastState = 1;
bool input_2_DownPin_lastState = 1;
bool input_1_UpPin_curState = 1;
bool input_1_DownPin_curState = 1;
bool input_2_UpPin_curState = 1;
bool input_2_DownPin_curState = 1;

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
  /*
    input == LOW is switch on
    rel == HIGH is switched off
  */

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
  bool st = HIGH;

  digitalWrite(output_1_UpPin, st);
  digitalWrite(output_1_DownPin, st);
  digitalWrite(output_2_UpPin, st);
  digitalWrite(output_2_DownPin, st);

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

void read_inputLastState() {
  input_1_DownPin_lastState = digitalRead(input_1_DownPin);
  input_1_UpPin_lastState = digitalRead(input_1_UpPin);
  input_2_UpPin_lastState = digitalRead(input_2_UpPin);
  input_2_DownPin_lastState = digitalRead(input_2_DownPin);
}

void check_input_1_pressedUp() {
  if (input_1_UpPin_curState != input_1_UpPin_lastState) {
    delay(debounceInt);
    if (digitalRead(input_1_UpPin) != input_1_UpPin_lastState) {
      if (digitalRead(output_1_DownPin) == HIGH) {
        digitalWrite(output_1_DownPin, LOW);
        digitalWrite(output_2_DownPin, LOW);
      }
      delay(deBounceInt);
      // ON OR OFF
      digitalWrite(output_1_UpPin, !input_1_UpPin_curState);
      digitalWrite(output_2_UpPin, !input_1_UpPin_curState); // update ESP pin

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
    if (digitalRead(input_1_DownPin) != input_1_DownPin_lastState) {
      if (digitalRead(output_1_UpPin) == HIGH) {
        digitalWrite(output_1_UpPin, LOW);
        digitalWrite(output_2_UpPin, LOW);
      }
      delay(deBounceInt);

      digitalWrite(output_1_DownPin, !input_1_DownPin_curState);
      digitalWrite(output_2_DownPin, !input_1_DownPin_curState); //update ESP pin

      Serial.print("DownLocal:");
      Serial.println(digitalRead(input_1_DownPin));
    }
  }

}

void check_input_2_pressedUp() {
  if (input_2_UpPin_curState != input_2_UpPin_lastState) {
    delay(debounceInt);
    if (digitalRead(input_2_UpPin) != input_2_UpPin_lastState) {
      if (digitalRead(output_1_DownPin) == HIGH) {
        digitalWrite(output_1_DownPin, LOW);
      }

      //                      ON OR OFF
      digitalWrite(output_1_UpPin, !input_2_UpPin_curState);

      Serial.print("UpRemote:");
      Serial.println(digitalRead(input_2_UpPin));
    }
  }

}

void check_input_2_pressedDown() {
  if (input_2_DownPin_curState != input_2_DownPin_lastState) {
    delay(debounceInt);
    if (digitalRead(input_2_DownPin) != input_2_DownPin_lastState) {
      if (digitalRead(output_1_UpPin) == HIGH) {
        digitalWrite(output_1_UpPin, LOW);
        // input_2_UpPin_lastState = input_2_UpPin_curState;
      }
      //                      ON OR OFF
      digitalWrite(output_1_DownPin, !input_2_DownPin_curState);
      // input_2_DownPin_lastState = !input_2_DownPin_curState;

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

void gpio_SWstatus() {
  Serial.print("up:");
  Serial.println(digitalRead(input_1_UpPin));
  Serial.print("down:");
  Serial.println(digitalRead(input_1_DownPin));
}

void gpio_RELstatus() {
  Serial.print("up:");
  Serial.println(digitalRead(output_1_UpPin));
  Serial.print("down:");
  Serial.println(digitalRead(output_1_DownPin));
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
  check_input_2_pressedDown();
  check_input_2_pressedUp();

  read_inputLastState();

  delay(50);
}
