#include <Arduino.h>
#include <Button2.h>

int pin = D6;
bool _state = false;
Button2 button;

void ON_OFF_on_handle(Button2 &b)
{
  Serial.println("ON");
}
void ON_OFF_off_handle(Button2 &b)
{
  b.getID();
  Serial.println("Off");
}
void start_ONOFFsw()
{
  button.setPressedHandler(ON_OFF_on_handle);
  button.setReleasedHandler(ON_OFF_off_handle);
}

void Momentary_handle(Button2 &b)
{
  _state = !_state;
  if (_state)
  {
    Serial.println("ON_momentary");
  }
  else
  {
    Serial.println("OFF_momentary");
  }
}
void start_MomentarySW()
{
  button.setPressedHandler(Momentary_handle);
}
void start_restartableSW()
{
}

void setup()
{
  Serial.begin(115200);
  button.begin(pin);
  Serial.println("\n\nStart");
  start_MomentarySW();
}

void loop()
{
  button.loop();
  // delay(100);
}
