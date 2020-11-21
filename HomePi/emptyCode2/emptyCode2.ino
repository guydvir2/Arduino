#include <myIOT.h>
#include "myIOT_settings.h"
#include <Arduino.h>

int swPin = 0;
int relayPin = 12;
int ledPin = 13;
//#include <myPIR.h>
//
//SensorSwitch radar0(D3);
//SensorSwitch radar1(D5);
//
//#define VER "WEMOS_0.2"
//// ~~~~~~~~~~~~~ 2.4GHz Radar ~~~~~~~~~~~~~~~
//int RadarsensorPin = D3;
//int RadarsensorPin2 = D5;
//
//bool scanRadar(const int reTrigger = 180, const int verifyDetectPeriod = 5, const int remainTriggered = 3, const int pin = 0)
//{
//        static bool lastState = false;
//        static bool det_notif = false;
//        static unsigned long last_det_clock = 0;
//        static unsigned long det_clock = 0;
//        /*seconds to be in detection mode - to avoid quick false alarms*/
//        /*seconds until declaring "end Detection" */
//        static int det_counter = 0;
//
//        bool state = digitalRead(pin);
//        if (state && det_notif == false)
//        {
//                if (det_clock == 0 && millis() > 1000UL * reTrigger + last_det_clock)
//                {
//                        /* first indication for detection */
//                        det_clock = millis();
//                        Serial.print("pre_detect-");
//                        Serial.println(pin);
//                        return 0;
//                }
//                else if (det_clock > 0 && millis() - det_clock > 1000UL * verifyDetectPeriod)
//                {
//                        /* Now it is a certified detection */
//                        Serial.println(pin);
//                        Serial.println("- Detection");
//                        char a[20];
//                        // sprintf(a, "Radar: detCount[#%d]", det_counter++);
//                        // iot.pub_msg(a);
//                        det_notif = true;
//                        det_clock = millis();
//                        return 1;
//                }
//        }
//        else if (state == false && det_clock > 0 && det_notif == false)
//        {
//                /* Clearing flags after a non-certified detection */
//                det_clock = 0;
//                Serial.println("clearing false alarm");
//                return 0;
//        }
//        else if (state == false && det_notif && millis() > remainTriggered * 1000UL + det_clock)
//        {
//                /* Ending detection mode, clearing flags */
//                Serial.println("END Detection");
//                det_notif = false;
//                det_clock = 0;
//                last_det_clock = millis();
//                return 0;
//        }
//}
//
//
//// ~~~~~~~~~~~ UltraSound Sensor ~~~~~~~~~~~~
//
//#define MAX_DISTANCE 250
//const int pingPin = D2; // Trigger Pin of Ultrasonic Sensor
//const int echoPin = D1; // Echo Pin of Ultrasonic Sensor
//
//void startGPIO()
//{
//        pinMode(pingPin, OUTPUT);
//        pinMode(echoPin, INPUT);
//}
//long microsecondsToCentimeters(long microseconds)
//{
//        return microseconds / 29 / 2;
//}
//int pingDistance()
//{
//        long duration, cm;
//
//        digitalWrite(pingPin, LOW);
//        delayMicroseconds(2);
//        digitalWrite(pingPin, HIGH);
//        delayMicroseconds(10);
//        digitalWrite(pingPin, LOW);
//        duration = pulseIn(echoPin, HIGH);
//        cm = microsecondsToCentimeters(duration);
//        if (cm > MAX_DISTANCE)
//        {
//                return 0;
//        }
//        else
//        {
//                return cm;
//        }
//}
//
//int checkChange(int tol = 3, float time_interval = 1.0, int re_trigger = 10)
//{
//        static unsigned int last_distance = 0;
//        static unsigned long last_check_clock = 0;
//        static unsigned long last_detection_clock = 0;
//        const int delay_between_pings = 500;
//
//        if (millis() > last_check_clock + (int)(time_interval * 1000))
//        {
//                unsigned int distance = pingDistance();
//                last_check_clock = millis();
//                if (distance != 0) /* not out of allowed detect range */
//                {
//                        int d = abs(distance - last_distance);
//                        if (d > tol)
//                        {
//                                delay(delay_between_pings);
//                                distance = pingDistance();
//                                d = abs(distance - last_distance);
//                                if (d > tol)
//                                {
//                                        last_distance = distance;
//                                        if (millis() > last_detection_clock + 1000 * re_trigger)
//                                        {
//                                                last_detection_clock = millis();
//                                                return d;
//                                        }
//                                        else
//                                        {
//                                                return 0;
//                                        }
//                                }
//                                else
//                                {
//                                        return 0;
//                                }
//                        }
//                        else
//                        {
//                                return 0;
//                        }
//                }
//                else
//                {
//                        return 0;
//                }
//        }
//        else
//        {
//                return 0;
//        }
//}
//void sonar_looper()
//{
//        int d = checkChange(3, 1.5);
//        if (d != 0)
//        {
//                iot.pub_msg("UltraSound detection");
//                Serial.println("AAAAA");
//        }
//}

void setup()
{
        startIOTservices();
        pinMode(swPin, INPUT);
        pinMode(ledPin, OUTPUT);
        pinMode(relayPin, OUTPUT);
        // startGPIO();
//        pinMode(RadarsensorPin, INPUT);
//        pinMode(RadarsensorPin2, INPUT);
}
void loop()
{
        iot.looper();
//        static int det_c = 0;
//        if (scanRadar(30, 5, RadarsensorPin) && scanRadar2(30, 5, RadarsensorPin2))
//        {
//                char a[30];
//                sprintf(a, "detect: det[#%d]", det_c++);
//        }

        // sonar_looper();
//        if (digitalRead(swPin, HIGH){
          digitalWrite(ledPin,digitalRead(swPin));
          digitalWrite(relayPin, digitalRead(swPin));
//        }
        delay(100);
}
