#include <LowPower.h>
volatile int sleep_counter = 0;

extern void exec_after_wakeup();

void sleepit_10sec(int x)
{
  if (sleep_counter < (x / 10))
  {
#if USE_SLEEP
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
#else
    delay(10000);
#endif
    sleep_counter++;
  }
  else
  {
    exec_after_wakeup();
    sleep_counter = 0;
  }
}
