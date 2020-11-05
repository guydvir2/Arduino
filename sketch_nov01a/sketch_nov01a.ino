#include <Arduino.h>
const int pingPin = D2; // Trigger Pin of Ultrasonic Sensor
const int echoPin = D1; // Echo Pin of Ultrasonic Sensor

long microsecondsToCentimeters(long microseconds)
{
   return microseconds / 29 / 2;
}
int pingDistance()
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
bool detectChange(int tol = 2, float time_interval = 1.0, const float correction_f = 0.04, const int delay_reping = 200)
{
   static int last_dist = 0;
   static unsigned long last_ping_clock = 0;

   while (millis() > last_ping_clock + 1000UL * time_interval)
   {
      int current_dist = pingDistance();
      int dist_0 = abs(last_dist - current_dist);
      last_ping_clock = millis();

      if (dist_0 * 1.0 >= max((float)tol, correction_f * current_dist))
      {
         delay(delay_reping);
         int current_dist2 = pingDistance();
         int dist_1 = abs(last_dist - current_dist2);
         if (dist_1 >= max((float)tol, correction_f * current_dist2))
         {
            // Serial.printf("updated_Distance:%d[cm]\n", current_dist);
            // Serial.print("change: ");
            // Serial.println(last_dist - current_dist);
            last_dist = current_dist;
            return true;
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
}
void detection_looper(int sec_re_trigger)
{
   static unsigned long re_trigger_clock = 0;
   
   if (millis() >= 1000UL * sec_re_trigger + re_trigger_clock)
   {
      if (detectChange(2, 0.2))
      {
         Serial.println("Detect");
         re_trigger_clock = millis();
      }
   }
}
void startGPIO(){
   pinMode(pingPin, OUTPUT);
   pinMode(echoPin, INPUT);
}
void setup()
{
   Serial.begin(9600);
   Serial.println("Start!");
   startGPIO();
}

void loop()
{
   detection_looper(10);
}
