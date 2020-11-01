#include <Arduino.h>

int sensorPin = 8;
bool lastState = false;
bool det_notif = false;
unsigned long det_clock = 0;
unsigned long last_det_clock = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Begin");
  pinMode(sensorPin, INPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  bool state = digitalRead(sensorPin);
  if (state && det_clock == 0&& millis()-last_det_clock>30000)
  {
    det_clock = millis();
  }
  else if (state && millis() - det_clock > 3000 && det_notif == false)
  {
    Serial.println("Detection");
    det_notif = true;
  }
  else if (state == false && det_notif == true)
  {
    Serial.println("ED - Detection");
    det_notif = false;
    det_clock = 0;
    last_det_clock = millis();
  }
  delay(200);
}
