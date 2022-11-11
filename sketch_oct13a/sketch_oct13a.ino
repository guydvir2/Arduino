#include <Arduino.h>
#include <myLOG.h>

flashLOG myLOG;

const char *a="guydvir";

int pins[] = {13, 14};

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  for (uint8_t i = 0; i < 2; i++)
  {
    pinMode(pins[i], INPUT_PULLUP);
  }
  bool x=a=="guydvir";
  Serial.println(x);
}
void loop()
{
//  char a[20];
//  sprintf(a, "%d;%d", digitalRead(pins[0]), digitalRead(pins[1]));
//  Serial.println(a);
}
