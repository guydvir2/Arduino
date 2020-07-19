#include <Servo.h>

Servo pitch_servo;
Servo rotate_servo;
#define pitchPin 8
#define rotatePin 9

struct servo_defs
{
  int pos;
  int min_angle;
  int max_angle;
  int PWMPin;
};

servo_defs pitch_defs = {120, 120, 180, pitchPin};
servo_defs rotate_defs = {90, 0, 180, rotatePin};

void init_servo()
{
  pitch_defs.pos = (int)(0.5 * (pitch_defs.min_angle + pitch_defs.max_angle));
  rotate_defs.pos = (int)(0.5 * (rotate_defs.min_angle + rotate_defs.max_angle));

  pitch_servo.attach(pitch_defs.PWMPin);   // attaches the servo on pin 9 to the servo object
  rotate_servo.attach(rotate_defs.PWMPin); // attaches the servo on pin 9 to the servo object

  Serial.print("Pitch start_angle: ");
  Serial.println(pitch_defs.pos);
  Serial.print("Rotate start_angle: ");
  Serial.println(rotate_defs.pos);
}

void slide(Servo &servo, servo_defs &def, int start_angle, int stop_angle, int del = 50)
{
  if (start_angle < def.min_angle)
  {
    start_angle = def.min_angle;
  }
  if (stop_angle > def.max_angle)
  {
    stop_angle = def.max_angle;
  }
  if (start_angle > stop_angle)
  {
    for (int x = start_angle; x >= stop_angle; x--)
    {
      servo.write(x);
      delay(del);
      Serial.println(x);
    }
  }
  else
  {
    for (int x = start_angle; x <= stop_angle; x++)
    {
      servo.write(x);
      delay(del);
      Serial.println(x);
    }
  }
}
void incMove(Servo &servo, servo_defs &def, int inc)
{
  Serial.print("increment by: ");
  Serial.print(inc);
  Serial.print(", Going to: ");
  Serial.println(def.pos + inc);

  if (def.pos + inc <= def.max_angle && def.pos + inc >= def.min_angle)
  {
    slide(servo, def, def.pos, def.pos + inc);
    def.pos += inc;
  }
  else if (def.pos + inc > def.max_angle)
  {
    slide(servo, def, def.pos, def.max_angle);
    def.pos = def.max_angle;
  }
  else if (def.pos + inc < def.min_angle)
  {
    slide(servo, def, def.pos, def.min_angle);
    def.pos = def.min_angle;
  }
}
void Move(Servo &servo, servo_defs &def, int deg)
{
  Serial.print("Moving to: ");
  Serial.println(deg);

  if (deg <= def.max_angle && deg >= def.min_angle)
  {
    slide(servo, def, def.pos, deg);
    def.pos = deg;
  }
  else if (deg > def.max_angle)
  {
    slide(servo, def, def.pos, def.max_angle);
    def.pos = def.max_angle;
  }
  else if (deg < def.min_angle)
  {
    slide(servo, def, def.pos, def.min_angle);
    def.pos = def.min_angle;
  }
}

int Pins[] = {A0, A1, A2, A3};
int analvals[] = {0, 0, 0, 0};

void setup()
{
  Serial.begin(9600);

  for (int a = 0; a < 4; a++)
  {
    pinMode(Pins[a], INPUT);
  }

  init_servo();
  slide(pitch_servo, pitch_defs, pitch_defs.pos, pitch_defs.max_angle);
  delay(1000);
  slide(rotate_servo, rotate_defs, rotate_defs.min_angle, rotate_defs.max_angle);
  delay(1000);
  // Move(pitch_servo, pitch_defs, 180);
  // Move(rotate_servo, rotate_defs, 90);
}

void loop()
{
  // slide(pitch_servo, pitch_defs, pitch_defs.min_angle, pitch_defs.max_angle);
  // delay(1000);
  // slide(pitch_servo, pitch_defs, pitch_defs.max_angle, pitch_defs.min_angle);
  // delay(1000);
  // slide(rotate_servo, rotate_defs, rotate_defs.min_angle, rotate_defs.max_angle);
  // delay(1000);
  // slide(rotate_servo, rotate_defs, rotate_defs.max_angle, rotate_defs.min_angle);
  // delay(1000);
  // for (int a = 0; a < 4; a++)
  // {
  //   analvals[a] = analogRead(Pins[a]);
  //   // Serial.print(analogRead(Pins[a]));
  //   // Serial.print("; ");
  // }
  // Serial.println("");

  const int diff = 20;
  const int deg_inc = 1;
  int delta_rot = analvals[0] - analvals[2];
  int delta_pitch = analvals[1] - analvals[3];
  // Serial.print("delta_rot: ");
  // Serial.println(delta_rot);
  // Serial.print("delta_pitch: ");
  // Serial.println(delta_pitch);
  // if (abs(delta_rot) > diff)
  // {
  //   Serial.print("Rot: ");
  //   Serial.println(delta_rot);
  //   if (delta_rot > diff)
  //   { // to much to the left
  //     incMove(rotate_servo, rotate_defs, deg_inc);
  //   }
  //   else
  //   {
  //     incMove(rotate_servo, rotate_defs, -deg_inc);
  //   }
  // }

  // if (abs(delta_pitch) > diff)
  // {
  //       Serial.print("pitch: ");
  //   Serial.println(delta_pitch);
  //   if (delta_pitch > diff)
  //   { 
  //     incMove(pitch_servo, pitch_defs, -deg_inc);
  //   }
  //   else
  //   {
  //     incMove(pitch_servo, pitch_defs, deg_inc);
  //   }
  // }

  // if (analvals[1] - analvals[3] > diff)
  // {
  //   incMove(pitch_servo, pitch_defs, 1);
  //   Serial.println("UP");
  // }
  // else if (analvals[1] - analvals[3] < diff)
  // {
  //   incMove(pitch_servo, pitch_defs, -1);
  //   Serial.println("DOWN");
  // }
  delay(10);

  Serial.println(pitch_servo.read());
}
