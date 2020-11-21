#include <avr/sleep.h>

void sleepFunction() {
  sleep_enable();     // set the sleep enable bit
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // attach the interrupt pin
  // use int0 pin => pin 2 on the arduino board
  // define which function to call if the interrupt is triggered
  // If you don't want to trigger on a falling edge you
  // need to change the third parameter
  attachInterrupt(0, interruptFunction, LOW);
  // some code ...
  sleep_cpu();        // put the arduino into sleep mode// code after wake up
}

void interruptFunction() {
  // some code ...
  sleep_disable();     // clear the sleep enable bit
  detachInterrupt(0);  // detatch the interrupt
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(" we are on");

}

void loop() {
  // put your main code here, to run repeatedly:

}
