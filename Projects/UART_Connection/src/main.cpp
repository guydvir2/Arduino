#include <Arduino.h>
char mystr[]="GUYDVIR";
void setup() {
  Serial.begin(9600);
    // put your setup code here, to run once:
}

void loop() {
  Serial.write(mystr,5);
  delay(1000);
    // put your main code here, to run repeatedly:
}
