#include <Arduino.h>

char a[3][30];

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  Serial.println(sizeof(a) / sizeof(a[0]));
}

void loop()
{

}
