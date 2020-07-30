#include <myPIR.h>
UltraSonicSensor USsensor(11, 10, 1, 20);

void detection_CB() {
  Serial.println("DETECT");
}
//USsensor.detect_cb(detection_CB);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BEGIN");
  USsensor.startGPIO();
}

void loop() {
  // put your main code here, to run repeatedly:
  USsensor.check_detect();
  delay(100);

}
