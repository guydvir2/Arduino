#include <Arduino.h>

// int outpin[] = {22, 23, 26, 27, 34, 36, 39, 35, 4, 15, 14, 2};
// int outpin[] = {22, 23, 26, 27, 4, 15, 14, 2};
int pin =25;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  pinMode(pin,OUTPUT);
  // for (uint8_t i = 0; i < sizeof(outpin) / sizeof(outpin[0]); i++)
  // {
  //   pinMode(outpin[i], INPUT_PULLUP);
  //   Serial.println(outpin[i]);
  // }
}
void loop()
{
  delay(1000);
  digitalWrite(pin,!digitalRead(pin));
  // for (uint8_t i = 0; i < sizeof(outpin) / sizeof(outpin[0]); i++)
  // {
  //   if (digitalRead(outpin[i]) == LOW)
  //   {
  //     Serial.println(outpin[i]);
  //   }
  // }
}