#include <myDisplay.h>
myOLED OLED(64);
void setup() {
  OLED.start();
  // put your setup code here, to run once:

}

void loop() {
  char a[20];
  OLED.char_size=3;
  sprintf(a,"%d",millis());
  // OLED.CenterTXT("HI GUY");
  OLED.freeTXT(a,"GUY    ",a,"  DVIR");
  delay(500);

  // put your main code here, to run repeatedly:

}
