#include <myIOT2.h>
#include <timeoutButton.h>
#include "myIOT_settings.h"

#define ButtonPin 15
#define TRIG_TYPE 0
#define TO_Button_ID 0

myIOT2 iot;
timeoutButton TO_Button_0;

void OFF_CB()
{
  Serial.println(TO_Button_0.remainWatch());
  Serial.println("OFF");
}
void ON_CB()
{
  Serial.println("ON");
  Serial.println(TO_Button_0.timeout);
}

void setup()
{
  startIOTservices();
  TO_Button_0.begin(ButtonPin, TRIG_TYPE, TO_Button_ID);
  TO_Button_0.ExtOFF_cb(OFF_CB);
  TO_Button_0.ExtON_cb(ON_CB);
  TO_Button_0.ON_cb(5);
}

void loop()
{
  TO_Button_0.loop();
  iot.looper();
}
