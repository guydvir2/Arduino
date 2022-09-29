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
  SW.set_input(19, 1);
  SW.set_output();
  SW.set_name("GUY_DVIR_SWITCH");
  SW.get_prefences();
}

void loop()
{
  if (SW.loop())
  {
    SW.clear_newMSG();
    Serial.print("NEW_MSG: State:");
    Serial.print(SW.telemtryMSG.state);
    Serial.print("\tReason: ");
    Serial.println(SW.telemtryMSG.reason);
  }
}
