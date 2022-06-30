#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

bool start_ads()
{
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    return 0;
  }
  else
  {
    return 1;
  }
}
float read_ads(uint8_t &channel)
{
  int16_t adc0;
  float volts;
  adc0 = ads.readADC_SingleEnded(channel);
  volts = ads.computeVolts(adc0);
  return volts;
}
float get_voltage(uint8_t x = 3, uint8_t channel = 0)
{
  float sum = 0;
  uint8_t i = 0;
  if (start_ads())
  {
    while (i < x)
    {
      sum += read_ads(channel);
      i++;
      delay(50);
    }
    return sum / ((float)x);
  }
  else
  {
    return -99.9;
  }
}