#include "Arduino.h"
int pin0 = D2;
bool READ;
bool &read = READ;

void printush(){
  Serial.println(read);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // pinMode(pin0, OUTPUT);
  pinMode(pin0, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // int x = 1;
  // int max_power = 1023;
  // analogWrite(pin0, max_power);
  // delay(1000);
  // analogWrite(pin0, max_power / 2);
  // delay(1000);
  // analogWrite(pin0, 0);
  // delay(1000);



  //  for (int i = 0; i < 1023; i=i+x) {
  //    analogWrite(pin0, i);
  //    delay(100);
  //    Serial.println(i);
  //  }
  //  delay(2000);
  //  for (int i = 1023; i > 0; i=i-x) {
  //    analogWrite(pin0, i);
  //    delay(100);
  //    Serial.println(i);
  //  }
  //
  //  delay(5000);
  // Serial.println(digitalRead(pin0));
  READ = digitalRead(pin0);
  printush();
  delay(200);
}
