#include <myPIR.h>

PIRsensor PIR(D1);
void detection(){
  Serial.println("DETECT");
}
void end_det(){
    Serial.println("endDETECT");
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("START!");
  PIR.use_timer = true;
  PIR.detect_cb(detection);
  PIR.end_detect_cb(end_det);

  PIR.start();

}

void loop() {
  // put your main code here, to run repeatedly:
PIR.looper();

  delay(200);



}
