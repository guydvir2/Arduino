#include <avr/sleep.h>
//
//
//void sleepFunction() {
//  sleep_enable();     // set the sleep enable bit
//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
//  // attach the interrupt pin
//  // use int0 pin => pin 2 on the arduino board
//  // define which function to call if the interrupt is triggered
//  // If you don't want to trigger on a falling edge you
//  // need to change the third parameter
//  attachInterrupt(0, interruptFunction, LOW);
//  // some code ...
////  Serial.println("ready to sleep");
////  Serial.flush();
//  sleep_cpu();        // put the arduino into sleep mode// code after wake up
//}
////
//void interruptFunction() {
//  // some code ...
//  sleep_disable();     // clear the sleep enable bit
//  detachInterrupt(0);  // detatch the interrupt
//}
////const byte ledPin = 17;
////const byte interruptPin = 2;
////volatile byte state = LOW;
////
////void setup() {
////  pinMode(ledPin, OUTPUT);
////  pinMode(interruptPin, INPUT_PULLUP);
////  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
////}
////
////void loop() {
////  digitalWrite(ledPin, state);
////}
////
////void blink() {
////  state = !state;
////}
//
//void setup() {
//  // put your setup code here, to run once:
//  Serial.begin(9600);
//  Serial.println("we are on");
//
//}
//
//void loop() {
//  // put your main code here, to run repeatedly:
////  if(millis()>10000){
////    sleepFunction();
////  }
//  delay(200);
//}

volatile byte changed = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2), toggle, CHANGE);
}

void loop() {
  if (changed == 1) {
    // toggle() has been called from interrupts!

    // Reset changed to 0
    changed = 0;

    // Blink LED for 200 ms
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void toggle() {
  changed = 1;
}
