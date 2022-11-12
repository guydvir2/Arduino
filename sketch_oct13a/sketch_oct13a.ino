#include <Arduino.h>

int outpin = D6;
int range = 1024;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  pinMode(outpin, OUTPUT);
}
void loop()
{
  for(int i=0; i<range; i++){
    analogWrite(outpin,i);
    Serial.println(i);
    delay(20);
  }
  delay(1000);
}
