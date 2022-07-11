#include <TurnOnLights.h>

#define PIN1 D6
#define PIN2 D7

TurnOnLights PWMled;
TurnOnLights ON_OFF_led;

void setup()
{
  PWMled.defStep = 2;
  PWMled.dimStep = 2;
  PWMled.dimDelay = 1;
  PWMled.maxSteps = 5;
  PWMled.limitPWM = 100;
  PWMled.init(PIN1, 1023, true);

  ON_OFF_led.init(PIN2, HIGH);
}

void loop()
{
  // for (uint8_t i = 1; i <= PWMled.maxSteps; i++)
  // {
  //   PWMled.turnON(i);
  //   ON_OFF_led.turnON();

  //   delay(500);
  //   PWMled.turnOFF();
  //   ON_OFF_led.turnOFF();
  //   delay(500);
  // }

  ON_OFF_led.blink(3,100);
  delay(2000);

}
