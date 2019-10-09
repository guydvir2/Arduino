#include <myIOT.h>
// #include <myJSON.h>
timeOUT TO("SW0",13);

void setup() {
Serial.begin(9600);
Serial.println("\nStarT");
TO.begin();
// TO.setNewTimeout(10, false);
// TO.store_dailyTO_inFlash(TO.dailyTO,0);
TO.check_dailyTO_inFlash(TO.dailyTO2,0);



}

void loop() {
  TO.looper();
  // Serial.println(TO.remain());
  delay(100);


}
