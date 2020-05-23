#include "LedControl.h"

LedControl lc = LedControl(11, 13, 10, 4);

void setup() {
  // put your setup code here, to run once:
  lc.shutdown(0, false); // Wake up displays
  lc.shutdown(1, false);
  lc.shutdown(2, false);
  lc.shutdown(3, false);
  lc.setIntensity(0, 5); // Set intensity levels
  lc.setIntensity(1, 5);
  lc.setIntensity(2, 5);
  lc.setIntensity(3, 5);
  lc.clearDisplay(0);  // Clear Displays
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  lc.clearDisplay(3);

}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 4; i++) {
    for (int r=0; r<8;r++){
      for (int c=0; c<8; c++){
         lc.setRow(i, 3, 3);
        delay(100);
      }
    }
  }
  lc.clearDisplay(0);  // Clear Displays
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  lc.clearDisplay(3);  delay(500);



}
