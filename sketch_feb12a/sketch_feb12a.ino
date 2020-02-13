/* This example code is in the public domain

  Tutorial page: https://arduinogetstarted.com/tutorials/arduino-motion-sensor
*/

const int PIN_TO_SENSOR = D1;   // the pin that OUTPUT pin of sensor is connected to
int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

void setup() {
  Serial.begin(9600);            // initialize serial
  pinMode(PIN_TO_SENSOR, INPUT); // set arduino pin to input mode to read value from OUTPUT pin of sensor

   digitalWrite(PIN_TO_SENSOR,0);

  // pinMode(3, OUTPUT);
  // digitalWrite(3,1);
}

void loop() {
   Serial.println(digitalRead(PIN_TO_SENSOR));
  // pinStatePrevious = pinStateCurrent; // store old state
  // pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state

  // if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // pin state change: LOW -> HIGH
  //   Serial.println("Motion detected!");
  //   // digitalWrite(3, 0);
  //   // delay(20);
  //   // digitalWrite(3, 1);


  //   // TODO: turn on alarm, light or activate a device ... here
  // }
  // else if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {  // pin state change: HIGH -> LOW
  //   Serial.println("Motion stopped!");
  //   // TODO: turn off alarm, light or deactivate a device ... here
  // }

  delay(200);
}
