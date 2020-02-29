#include <Servo.h>

Servo pitch_servo;
Servo rotate_servo;

struct servo_defs
{
  int pos;
  int min_angle;
  int max_angle;
  int PWMPin;
};

servo_defs pitch_defs = {120, 120, 180, 6};
servo_defs rotate_defs = {0, 0, 180, 9};

void init_servo()
{
  pitch_defs.pos = pitch_servo.read();
  rotate_defs.pos = rotate_servo.read();
  Serial.print( "Pitch start angle: ");
  Serial.println(pitch_defs.pos);
  Serial.print("Rotate start angle: ");
  Serial.println(rotate_defs.pos);
}

void slide(Servo &servo, int start, int stop, int del=50)
{
  if (start > stop)
  {
    for (int x = start; x >= stop; x--)
    {
      servo.write(x);
      delay(del);
      Serial.println(x);
    }
  }
  else
  {
    for (int x = start; x <= stop; x++)
    {
      servo.write(x);
      delay(del);
      Serial.println(x);
    }
  }
}



void setup()
{
  Serial.begin(9600);
  pitch_servo.attach(pitch_defs.PWMPin);   // attaches the servo on pin 9 to the servo object
  rotate_servo.attach(rotate_defs.PWMPin); // attaches the servo on pin 9 to the servo object

  init_servo();
  slide(pitch_servo,pitch_defs.pos, pitch_defs.min_angle);
  delay(1000);
  slide(rotate_servo, rotate_defs.pos, rotate_defs.min_angle);
  delay(1000);
}

void loop()
{
  slide(pitch_servo, pitch_defs.min_angle, pitch_defs.max_angle);
  delay(1000);
  slide(pitch_servo, pitch_defs.max_angle, pitch_defs.min_angle);
  delay(1000);
  slide(rotate_servo, rotate_defs.min_angle, rotate_defs.max_angle);
  delay(1000);
  slide(rotate_servo, rotate_defs.max_angle, rotate_defs.min_angle);
  delay(1000);

}
