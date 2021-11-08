#include <buttonPresses.h>

#define BUTTONPRESSED LOW
#define LEDON HIGH

const int ledPin = 4;
const int butPin = 8;
const int butPin2 = 12;
// buttonPresses buttA(butPin, 2, butPin2);
buttonPresses buttA(butPin, 0);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  buttA.start();

  pinMode(ledPin, OUTPUT);
}

void loop()
{
  uint8_t a = buttA.read();

  if (a != 8)
  {
    Serial.println(a);
  }
  delay(500);
}