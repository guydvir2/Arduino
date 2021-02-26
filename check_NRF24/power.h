/* 
This Power section is relevant 
only to remote sleeping unit (and conststs of:
1. Powering Up/Down RF unit using a relay or a enabled DC-DC converter 
2. Sleep Section
3. Measuring VBat using analog Read of A0
 at current time it is a Pro- Micro )
*/
#include <LowPower.h>

// ~~~~~~~~~ Control Power to RF Module ~~~~~~~~~~~
const byte relPin = 2;
const bool POW_ON = HIGH;
const bool POW_OFF = !POW_ON;

void start_relay()
{
  pinMode(relPin, OUTPUT);
  digitalWrite(relPin, POW_OFF);
}
void power_RF(bool state)
{
  if (state)
  {
    digitalWrite(relPin, POW_ON);
    delay(100);
    radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  }
  else
  {
    digitalWrite(relPin, POW_OFF);
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ Measure Vbat via A0 ~~~~~~~~~~~~~
const byte ADCPin = A0;
const float Vref = 5.08; // VCC via DC-DC Boost

float measure_VBAT(const byte x = 5)
{
  int y = 0;
  float vbat = 0;
  for (int i = 0; i < x; i++)
  {
    y += analogRead(ADCPin);
    delay(10);
  }
  vbat = (float)y / (float)x;
  vbat /= 1024;
  vbat *= Vref;
  return vbat;
}
char *get_vbatC()
{
  char *str_temp = new char[6];
  float v = measure_VBAT();
  dtostrf(v, 4, 2, str_temp);
  strcat(str_temp, "[v]");
  return str_temp;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ Sleep ~~~~~~~~~~~~~~~~~~~~~~~~~~~
volatile unsigned int sleep_counter = 0;

void exec_after_wakeup()
{
/* state here what to execute after wake up */
#if PRINT_MESSAGES
  Serial.println("WakeUp!");
  delay(10);
  Serial.flush();
#endif
  power_RF(HIGH); /* Power UP RF module */
  char a[8];
  char c[8];
  char *b = get_vbatC();
  sprintf(a, "%d", sleep_counter);
  sprintf(c, "%d", SLEEP_TIME);
  send(m_types[3], a, b, c, wakes);
}
void callsleep_period(int x)
{
  int coeffs[3] = {0, 0, 0};

  coeffs[0] = x / 8;                             /* 8 sec */
  coeffs[1] = (x - 8 * coeffs[0]) / 2;           /* 2 sec */
  coeffs[2] = x - 8 * coeffs[0] - 2 * coeffs[1]; /* 1 sec */

  for (int x = 0; x < 3; x++)
  {
    if (coeffs[x] != 0)
    {
      for (int c = 0; c < coeffs[x]; c++)
      {
        switch (x)
        {
        case 0:
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
          break;
        case 1:
          LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
          break;
        case 2:
          LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
          break;
        default:
          break;
        }
      }
    }
  }
  sleep_counter++;
  exec_after_wakeup();
  // ~~~~~~~~~~~~~~~~~~~
}
void sleep_cycle(const int sleep_time = 60, const byte no_sleep_time = 10)
{
  static unsigned long last_loop = 0;
  if (millis() - last_loop > no_sleep_time * 1000UL)
  {
#if PRINT_MESSAGES
    Serial.println("Zzz");
    delay(10);
    Serial.flush();
#endif
    last_loop = millis();
    power_RF(LOW);
    callsleep_period(sleep_time);
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~