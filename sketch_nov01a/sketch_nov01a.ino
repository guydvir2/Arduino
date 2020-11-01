#include <Arduino.h>
const int pingPin = D2; // Trigger Pin of Ultrasonic Sensor
const int echoPin = D1; // Echo Pin of Ultrasonic Sensor

int getDistance()
{
   long duration, cm;

   digitalWrite(pingPin, LOW);
   delayMicroseconds(2);
   digitalWrite(pingPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(pingPin, LOW);
   duration = pulseIn(echoPin, HIGH);
   cm = microsecondsToCentimeters(duration);
   return cm;
}
bool detectChange(int tol = 2, int time_interval=1)
{
   static int last_dist = 0;
   static unsigned long last_check = 0;

   while (millis() > last_check + 1000UL * time_interval)
   {
      int current_dist = getDistance();
      last_check = millis();
      if (abs(last_dist - current_dist) >= tol || (float)abs(last_dist - current_dist)>(float)(last_dist)*1.025)
      {
         Serial.printf("updated_Distance:%d[cm]\n", current_dist);
         last_dist = current_dist;

         return true;
      }
      else
      {
         return false;
      }
   }
}
void setup()
{
   Serial.begin(9600);
   Serial.println("Start!");
   pinMode(pingPin, OUTPUT);
   pinMode(echoPin, INPUT);
}

void loop()
{
   detectChange();
}

long microsecondsToCentimeters(long microseconds)
{
   return microseconds / 29 / 2;
}
