#include <myIOT.h>
#include <Arduino.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/Pergola_ledStrips"
#define ADD_MQTT_FUNC addiotnalMQTT
#define VER "SONOFFsw_2.7"

#define ON_AT_BOOT false
#define USE_SERIAL false
#define USE_WDT true
#define USE_OTA true
#define USE_MAN_RESET false
#define USE_BOUNCE_DEBUG false
#define USE_TIMEOUT false
#define TIME_OUT_VALUE 30
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
long startTime = 0;


myIOT iot(DEVICE_TOPIC);

void setup() {
  startGPIOs();

  if (ON_AT_BOOT == true ) {
    digitalWrite(outputPin, RelayOn);
  }
  else {
    digitalWrite(outputPin, digitalRead(inputPin));
  }

  if ( USE_TIMEOUT == true ) { // boot into timeout-mode
    digitalWrite(outputPin, RelayOn);
    startTime = millis();
  }

  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.start_services(ADD_MQTT_FUNC);
 if ( USE_TIMEOUT == true ) {
  switchIt("TimeOut", "on");
 }
}
void startGPIOs() {
  pinMode(inputPin, INPUT_PULLUP);
  pinMode(inputLocalPin, INPUT_PULLUP);
  pinMode(outputPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  inputLocalPin_lastState = digitalRead(inputLocalPin);
  inputPin_lastState = digitalRead(inputPin);
}
void addiotnalMQTT(char incoming_msg[50]) {
  char state[5];
  char state2[5];
  char msg[100];
  char msg2[100];

  if (strcmp(incoming_msg, "status") == 0) {
    // relays state
    if (digitalRead(outputPin) == RelayOn) {
      sprintf(state, "On");
    }
    else if (digitalRead(outputPin) == !RelayOn ) {
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
    sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], MAN_RESET:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_MAN_RESET);
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
    digitalWrite(outputPin, RelayOn);
  }
  else if (strcmp(dir, "off") == 0) {
    digitalWrite(outputPin, !RelayOn);
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
      if (USE_BOUNCE_DEBUG) {
        char tMsg [100];
        sprintf(tMsg, "Bounce RemInput: cRead [%d] lRead[%d]", digitalRead(inputPin), inputPin_lastState);
        iot.pub_msg(tMsg);
      }
    }
  }
}
void checkLocalInput() {
  if (digitalRead(inputLocalPin) == SwitchOn) {
    delay(deBounceInt);
    if (digitalRead(inputLocalPin) == SwitchOn ) {
      if (digitalRead(outputPin) == RelayOn ) {
        switchIt("localButton", "off");
      }
      else {
        switchIt("localButton", "on");
      }
      while (digitalRead(inputLocalPin) == SwitchOn) {
        delay(50);
      }
    }
    else { // for debug only
      if (USE_BOUNCE_DEBUG) {
        char tMsg [100];
        sprintf(tMsg, "Bounce LocInput: cRead [%d] lRead[%d]", digitalRead(inputPin), inputPin_lastState);
        iot.pub_msg(tMsg);
      }
    }
  }
}
void readGpioStates() {
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

  if (USE_TIMEOUT == true && startTime !=0 ) {
    if ((millis() - startTime ) > TIME_OUT_VALUE * 1000 * 60 ) {
      switchIt("TimeOut", "off");
      startTime = 0;
    }
  }

  delay(150);
}
