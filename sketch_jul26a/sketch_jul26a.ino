// class Actions
// {
//   typedef void (*cb_func)(uint8_t resaon);

// public:
//   cb_func exter_func;

// public:
//   void get_external_func(cb_func func)
//   {
//     exter_func = func;
//   }
//   void callAction(uint8_t i)
//   {
//     exter_func(i);
//   }
// };

// class Triggers
// {
// public:
//   static void this_is_external_func(uint8_t i)
//   {
//     Serial.println(i);
//   }
// };

// class Combine : public Actions, Triggers
// {
//   typedef void (*cb_func)(uint8_t resaon);

// public:
//   void combine_funcs(cb_func func)
//   {
//     // get_external_func(this_is_external_func);
//     // exter_func = (void *)this_is_external_func;
//     exter_func = func;
//   }
//   void begin()
//   {
//     combine_funcs(this_is_external_func);
//   }
//   void loop()
//   {
//     if (millis() > 10000)
//     {
//       callAction(13);
//     }
//   }
// };

// Combine combo;

#include <timeoutButton.h>

LightButton Lightbut;
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start!");

  Lightbut.dimmablePWM = true;
  Lightbut.output_ON = HIGH;
  Lightbut.inputPressed = LOW;
  Lightbut.outputPin = D6;
  Lightbut.inputPin = D3;
  Lightbut.indicPin = D7; /* set to 255 to disable */

  Lightbut.def_TO_minutes = 360;
  Lightbut.maxON_minutes = 1000;

  Lightbut.defPWM = 2;
  Lightbut.max_pCount = 3;
  Lightbut.limitPWM = 50;
  Lightbut.PWM_res = 1023;

  Lightbut.begin(0);
}

void loop()
{
  // put your main code here, to run repeatedly:
  Lightbut.loop();
}
