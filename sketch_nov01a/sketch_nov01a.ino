#include <Arduino.h>
#include <NewPing.h>

#define TRIGGER_PIN D2
#define ECHO_PIN D1
#define MAX_DISTANCE 250

// NewPing setup of pins and maximum distance
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// const int pingPin = D2; // Trigger Pin of Ultrasonic Sensor
// const int echoPin = D1; // Echo Pin of Ultrasonic Sensor

// long microsecondsToCentimeters(long microseconds)
// {
//    return microseconds / 29 / 2;
// }
// int pingDistance()
// {
//    long duration, cm;

//    digitalWrite(pingPin, LOW);
//    delayMicroseconds(2);
//    digitalWrite(pingPin, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(pingPin, LOW);
//    duration = pulseIn(echoPin, HIGH);
//    cm = microsecondsToCentimeters(duration);
//    return cm;
// }
// bool detectChange(int tol = 2, float time_interval = 1.0, const float correction_f = 0.04, const int delay_reping = 200)
// {
//    static int last_dist = 0;
//    static unsigned long last_ping_clock = 0;

//    while (millis() > last_ping_clock + 1000UL * time_interval)
//    {
//       int current_dist = pingDistance();
//       int dist = abs(last_dist - current_dist);
//       last_ping_clock = millis();

//       if (dist * 1.0 >= max((float)tol, correction_f * current_dist))
//       {
//          delay(delay_reping);
//          int current_dist2 = pingDistance();
//          int dist_1 = abs(last_dist - current_dist2);
//          if (dist_1 >= max((float)tol, correction_f * current_dist2))
//          {
//             // Serial.printf("updated_Distance:%d[cm]\n", current_dist);
//             // Serial.print("change: ");
//             // Serial.println(last_dist - current_dist);
//             last_dist = current_dist;
//             return true;
//          }
//          else
//          {
//             return false;
//          }
//       }
//       else
//       {
//          return false;
//       }
//    }
// }
bool detectChange(int tol = 2, float time_interval = 1.0, const float correction_f = 0.04, const int delay_reping = 500)
{
   static int last_dist = 0;
   static unsigned long last_ping_clock = 0;

   while (millis() > last_ping_clock + 1000UL * time_interval)
   {
      int current_dist = sonar.ping_cm();
      int dist = abs(last_dist - current_dist);
      last_ping_clock = millis();
      if (dist != 0)
      {

         if (dist * 1.0 >= max((float)tol, correction_f * current_dist))
         {
            delay(delay_reping);
            current_dist = sonar.ping_cm();
            dist = abs(last_dist - current_dist);
            if (dist >= max((float)tol, correction_f * current_dist))
            {
               // Serial.printf("updated_Distance:%d[cm]\n", current_dist);
               // Serial.print("change: ");
               // Serial.println(last_dist - current_dist);
               last_dist = current_dist;
               Serial.println(last_dist);
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
      else
      {
         return 0;
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

// void startGPIO()
// {
//    pinMode(pingPin, OUTPUT);
//    pinMode(echoPin, INPUT);
// }

int checkChange(int tol = 3, float time_interval = 1.0, int re_trigger = 20)
{
   static unsigned int last_distance = 0;
   static unsigned long last_check_clock = 0;
   static unsigned long last_detection_clock = 0;
   const int delay_between_pings = 500;

   if (millis() > last_check_clock + (int)(time_interval * 1000))
   {
      unsigned int distance = sonar.ping_cm();
      last_check_clock = millis();
      if (distance != 0) /* not out of allowed detect range */
      {
         int d = abs(distance - last_distance);
         if (d > tol)
         {
            delay(delay_between_pings);
            distance = sonar.ping_cm();
            d = abs(distance - last_distance);
            if (d > tol)
            {
               last_distance = distance;
               if (millis() > last_detection_clock + 1000UL * re_trigger)
               {
                  last_detection_clock = millis();
                  return d;
               }
               else
               {
                  return 0;
               }
            }
            else
            {
               return 0;
            }
         }
         else
         {
            return 0;
         }
      }
      else
      {
         return 0;
      }
   }
   else
   {
      return 0;
   }
}
void setup()
{
   Serial.begin(9600);
   Serial.println("Start!");
   // startGPIO();
}

void loop()
{
   int d = checkChange(3, 1.5);
   if (d != 0)
   {
      Serial.println(d);
   }
   // detection_looper(15);
   delay(100);
}
