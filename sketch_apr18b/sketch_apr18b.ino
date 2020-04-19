#include <myIOTesp32.h>

myIOT32 iot();
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nSTART");
  // iot.client.enableLastWillMessage("TestClient/lastwill", "I am going offline");

}

void loop() {
  // put your main code here, to run repeatedly:
  iot.looper();

}
