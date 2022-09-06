#include <Arduino.h>



char *a[2] = {nullptr, nullptr};

char b[20];

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  sprintf(b, "dfgsfgsdfgsdfg");

  a[0] = b;

  // for (uint8_t i = 0; i < 10; i++)
  // {
    Serial.println(a[0]);
  // }
}

void loop()
{
}
