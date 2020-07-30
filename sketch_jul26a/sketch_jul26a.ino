#include <myJSON.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nBegin");
  myTest mt;
  mt.funcOne(123.5);

}

void loop() {
  // put your main code here, to run repeatedly:

}
