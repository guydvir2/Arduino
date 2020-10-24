#include <Arduino.h>
#define SEC_TO_APPROVE_DETECT 4

int sensPin = D2;
bool detect = false;
long firstDetect_clock = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BEGIN");
  pinMode(sensPin, INPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (detect == false && digitalRead(sensPin) == true)
  {
    if (firstDetect_clock == 0)
    {
      firstDetect_clock = millis();
      Serial.println("gong");
    }
    else if (millis() - firstDetect_clock > 1000 * SEC_TO_APPROVE_DETECT)
    {
      Serial.println("Detect");
      detect = true;
    }
  }
  else if (detect == false && digitalRead(sensPin) == false && firstDetect_clock > 1000 * SEC_TO_APPROVE_DETECT)
  {
    firstDetect_clock = 0;
    Serial.println("Zero- no activity");
  }
  else if (detect == true && digitalRead(sensPin) == false)
  {
    detect = false;
    firstDetect_clock = 0;
    Serial.println("END_Detect");
  }
  delay(100);
}
