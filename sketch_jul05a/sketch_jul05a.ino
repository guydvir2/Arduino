#include <timeoutButton.h>
timeoutButton button;

#define BUTTON_PIN D5
#define TRIG_TYPE 2 // 0,1,2
#define BUTTON_ID 0

void ON_CB(uint8_t a)
{
  Serial.println("ON");
}
void OFF_CB(uint8_t a)
{
  Serial.println("OFF");
}
void MULTP_CB(uint8_t reason)
{
  Serial.println(reason);
}

void start_button()
{
#if TRIG_TYPE == 2
  button.ExtMultiPress_cb(MULTP_CB);
#else
  button.ExtON_cb(ON_CB);
  button.ExtOFF_cb(OFF_CB);
#endif
  button.begin();//BUTTON_PIN, TRIG_TYPE, BUTTON_ID);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nBegin~");
  start_button();
}

void loop()
{
  button.loop();
  if (button.getState ()> 0)
  {
    Serial.println(button.remainWatch());
  }
  delay(100);
}
