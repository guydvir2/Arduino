#include <buttonPresses.h>

buttonPresses buttA(D1, 0);     /* Button */
buttonPresses buttB;            /* Parameters can be defined in setup() */
buttonPresses buttC(D6, 2, D5); /* 3 state Rocker Switch */
buttonPresses buttD(D7, 3);     /* Multi-press button */

void setup()
{
  Serial.begin(115200);
  buttB.pin0 = D3;
  buttB.buttonType = 1;
  buttB.start();
}

void loop()
{
  uint8_t b = buttB.read(); /* Each check take 50 ms */
  uint8_t c = buttC.read();
  uint8_t d = buttD.read();

  if (buttA.read() != 0)
  {
    Serial.println("Button Press");
  }
  if (b != 0)
  {
    if (b == 1)
    {
      Serial.println("Switch ON");
    }
    else if (b == 2)
    {
      Serial.println("Switch OFF");
    }
  }
  if (c != 0)
  {
    if (c == 1)
    {
      Serial.println("Switch UP");
    }
    else if (c == 2)
    {
      Serial.println("Switch DOWN");
    }
    else if (c == 3)
    {
      Serial.println("Switch OFF");
    }
  }
  if (d != 0)
  {
    if (d != buttD.END_LONG_PRESS_VAL)
    {
      Serial.print("Presses: ");
      Serial.println(d);
    }
    else
    {
      Serial.println("Long Press!");
    }
  }
}
