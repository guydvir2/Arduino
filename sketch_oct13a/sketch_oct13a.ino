#include <Arduino.h>
#include <smartSwitch.h>

smartSwitch sw;

char *func()
{
  return sw.ver;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  delay(2000);
}
void loop()
{
  // sw_v[0]->loop();
  // Serial.println(sw_v[0]->telemtryMSG.state);
  Serial.println(func());
  delay(200);
}
