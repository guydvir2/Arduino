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



void setup() {
  Serial.begin(9600);
  pitch_servo.attach(pitch_defs.PWMPin);  // attaches the servo on pin 9 to the servo object
  rotate_servo.attach(rotate_defs.PWMPin);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  for (int a = pitch_defs.min_angle; a<=pitch_defs.max_angle; a++){
    pitch_servo.write(a);
    delay(10); 
  }
  delay(2000);
  for (int a = rotate_defs.min_angle; a<=rotate_defs.max_angle; a++){
    rotate_servo.write(a);
    delay(10); 
  }
  delay(5000);
  Serial.println("LIIP");
  //  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
  //    // in steps of 1 degree
  //    myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //    delay(15);                       // waits 15ms for the servo to reach the position
  //  }
  //  delay(1000);
  //  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
  //    myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //    delay(15);                       // waits 15ms for the servo to reach the position
  //  }
}
