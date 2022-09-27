#include <Arduino.h>
#include <smartSwitch.h>

smartSwitch SW;

void smartSwitch::set_extON(uint8_t opt)
{
  Serial.println(opt);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  SW.set_input(D3, 2);
  SW.set_output(D4);
  SW.set_name("GUY_DVIR_SWITCH");
  SW.get_prefences();
}

void loop()
{
  SW.loop();
}
