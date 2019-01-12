#include <iot.h>

// GPIO Pins for ESP8266
const int inputUpPin = 4;
const int inputDownPin = 5;
const int outputUpPin = 14;
const int outputDownPin = 12;
//##########################

iot iot_dev;

void setup() {
  //  Serial.begin(9600);
  //  startGPIOs();



  // put your setup code here, to run once:

}

void startGPIOs() {
  // <------------ Need to be changed
  pinMode(inputUpPin, INPUT_PULLUP);
  pinMode(inputDownPin, INPUT_PULLUP);
  pinMode(outputUpPin, OUTPUT);
  pinMode(outputDownPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //  Serial.println(morse.run());
  //  iot_dev.network_check();
  delay(100);


}
