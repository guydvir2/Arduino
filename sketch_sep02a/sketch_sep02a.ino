#include <buttonPresses.h>

buttonPresses buttSwitch;
buttonPresses *buttSwitchEXT[] = {nullptr, nullptr};

#define RELAY_ON LOW
#define SW_PRESSED LOW
#define SW_DOWN_PIN D1   //4  /* Switch1 INPUT to Arduino */
#define SW_UP_PIN D2     //5  /* Switch1 INPUT to Arduino */
#define REL_DOWN_PIN D3 //3  /* OUTUPT to relay device */
#define REL_UP_PIN D4   //2  /* OUTUPT to relay device */
bool DUAL_SW = false;

void start_buttSW()
{
  buttSwitch.pin0 = SW_UP_PIN;
  buttSwitch.pin1 = SW_DOWN_PIN;
  buttSwitch.buttonType = 2;
  buttSwitch.start();

  if (DUAL_SW)
  {
    static buttonPresses buttSwitchExt;
    buttSwitchEXT[0] = &buttSwitchExt;
    // buttSwitchEXT[0]->pin0 = SW2_UP_PIN;
    // buttSwitchEXT[0]->pin1 = SW2_DOWN_PIN;
    buttSwitchEXT[0]->buttonType = 2;
    buttSwitchEXT[0]->start();
  }
}
void read_buttSwitch()
{
  uint8_t switchRead = buttSwitch.getValue();
  if (switchRead != 0)
  {
    Serial.println(switchRead);
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nbegin");
  start_buttSW();
}

void loop()
{
  // put your main code here, to run repeatedly:
  read_buttSwitch();
  delay(100);
}
