#include <myIOT.h>
#include <Arduino.h>

flashLOG flog;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  flog.start();
  flog.write("HITHERE");
  flog.read();

}

void loop() {
  // put your main code here, to run repeatedly:

}
