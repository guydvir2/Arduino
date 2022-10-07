#include <Arduino.h>
#include <myWindowSW.h>

RockerSW SW;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  SW.set_input(19,15);
  }
void loop()
{
Serial.println(SW.get_SWstate());
// SW.get_SWstate();
delay(500);
}
