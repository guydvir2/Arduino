#include <Arduino.h>
// #include <ESP8266WiFi.h>

int outpin = D1;
int outpin2 = D2;
int range = 1023;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  // analogWriteRange(range);

  pinMode(outpin, OUTPUT);
  pinMode(outpin2, OUTPUT);
}
void loop()
{
  // digitalWrite(outpin, !digitalRead(outpin));
  for (int i = 0; i < 255; i++)
  {
    analogWrite(outpin, i);
    analogWrite(outpin2, i);
    delay(20);
    Serial.println(i);
  }
  delay(1000);
}