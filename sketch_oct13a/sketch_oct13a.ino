#include <Arduino.h>
#include<myJflash.h>


myJflash Jflash;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  Jflash.set_filename("/guy.txt");
}
void loop()
{
  delay(1000);
}