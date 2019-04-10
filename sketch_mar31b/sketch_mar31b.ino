#define USE_SLEEP true

#if USE_SLEEP
#include <avr/sleep.h>
#endif

#define interruptPin_1 2
#define interruptPin_2 3
#define RelayON HIGH
#define doorOpen HIGH

volatile boolean detectDoor_1_Open = false;
volatile boolean detectDoor_2_Open = false;
const byte relayPin1               = 4;
const byte relayPin2               = 5;


void setup() {
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(interruptPin_1, INPUT);
  pinMode(interruptPin_2, INPUT);
  attachInterrupt(0, detection_door, CHANGE);
  attachInterrupt(1, detection_door, CHANGE);
}

void detection_door() {
#if USE_SLEEP
  sleep_disable();
#endif

  detachInterrupt(0);
  detachInterrupt(1);

  if (digitalRead(interruptPin_1) == doorOpen) { //door1
    detectDoor_1_Open = true;
  }
  else {// trun off
    detectDoor_1_Open = false;
  }

  if (digitalRead(interruptPin_2) == doorOpen) { //door2
    detectDoor_2_Open = true;
  }
  else {// trun off
    detectDoor_2_Open = false;
  }
}

void go2sleep() {
#if USE_SLEEP
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
#endif

}

void loop() {
  if (detectDoor_1_Open == true ) {
    digitalWrite(relayPin1, RelayON);
  }
  else {
    digitalWrite(relayPin1, !RelayON);
  }

  if (detectDoor_2_Open == true ) {
    digitalWrite(relayPin2, RelayON);
  }
  else {
    digitalWrite(relayPin2, !RelayON);
  }
  attachInterrupt(0, detection_door, CHANGE);
  attachInterrupt(1, detection_door, CHANGE);

#if USE_SLEEP
  go2sleep();
#endif
}
