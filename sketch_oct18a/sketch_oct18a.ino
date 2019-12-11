#include <Arduino.h>
int pwmPin = D3;
int maxValue = 1023;
int del = 30;
double inc = 0.02;
double x = 0;

void setup()
{
        pinMode(pwmPin, OUTPUT);
        analogWrite(pwmPin, 1023);
}

void loop()
{
        // Serial.println("0");
        // analogWrite(pwmPin, 0);
        // delay(1000);
        // Serial.println("1/3");
        // analogWrite(pwmPin, maxValue/3);
        // delay(1000);
        // Serial.println("1/2");
        // analogWrite(pwmPin, maxValue/2);
        // delay(1000);
        // Serial.println("3/4");
        // analogWrite(pwmPin, maxValue*0.75);
        // delay(1000);
        // Serial.println("1");
        // analogWrite(pwmPin, maxValue);
        // delay(1000);
        // delay(del);
        // if (x <= maxValue + inc * maxValue)
        // {
        //         x = x + inc * maxValue;
        // }
        // else
        // {
        //         x = 0;
        //         delay(2000);
        // }
        // analogWrite(pwmPin, x);
}
