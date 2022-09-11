#include <Arduino.h>
#define PIN_1 1
#define PIN_2 2

#define COND true

#define POWEROFF           \
  if (COND)                \
  digitalWrite(PIN_1, LOW) \
      digitalWrite(PIN_2, LOW)

#define winUP \
  if (COND)   \
    POWEROFF  \
  digitalWrite(PIN_2, LOW)

#define winDOWN \
  if (COND)     \
    POWEROFF    \
  digitalWrite(PIN_1, LOW)

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
}

void loop()
{
}
