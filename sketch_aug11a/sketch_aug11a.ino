#include <Time.h>
time_t t=now();
//setTime(15,20,0,7,8,2017);
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println(millis());
Serial.println(t);

delay(1000);
}
