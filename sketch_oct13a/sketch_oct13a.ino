#include <Arduino.h>

char a[4][20];
char *b[4];

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");

  sprintf(a[0], "THIS IS MY FIRST");
  b[0] = a[0];

  Serial.println(a[0]);
  Serial.println(b[0]);

  sprintf(a[0], "THIS IS MY SECOND");

  Serial.println(a[0]);
  Serial.println(b[0]);
}
void loop()
{

}
