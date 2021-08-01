#include <buttonPresses.h>

buttonPresses buttA(D1, 3); /* Button */
// buttonPresses buttB;         /* Parameters can be defined in setup() */
// buttonPresses buttC(D6,2,D5); /* 3 state Rocker Switch */

void setup()
{
  Serial.begin(115200);
  // buttB.pin0 = D3;
  // buttB.buttonType = 1;
  // buttB.start();
}

void loop()
{
  buttA.getValue();
  // buttB.getValue();
  // buttC.getValue();
}
