#include <Wire.h>
#include <Adafruit_ADS1015.h>

float ADC_measures[2] = {0, 0};
float v_div_coeff[2] = {1.68, 2.57}; /* Voltage divider*/
float correction_f[2] = {0.6, 11.25}; /* measure errorsq inaccuracy such diodes reduces 0.6v */

Adafruit_ADS1115 ads;

void startADC()
{
  ads.begin();
}
float get_Vmeasure(byte i, byte samSize = 5, byte del = 5)
{
  ADC_measures[i] = 0;
  for (int x = 0; x < samSize; x++)
  {
    ADC_measures[i] += ads.readADC_SingleEnded(i);
    delay(del);
  }
  ADC_measures[i] /= samSize;

  return correction_f[i] + 0.1875 * ADC_measures[i] * v_div_coeff[i] * 0.001; /* Reply as [v] */
}
