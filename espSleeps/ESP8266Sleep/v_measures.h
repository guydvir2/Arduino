#include <Arduino.h>
#include <Adafruit_ADS1015.h>

float vbat = 0.0;
float vsolarpannel = 0.0;

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
Adafruit_ADS1115 ads;

void startADS()
{
  if (vmeasure_type == 1)
  {
    ads.begin();
    // ads.setGain(GAIN_TWOTHIRDS); // 0.1875mV (default)
  }
}
float get_vmeasure_ADS(const byte x = 0)
{
  if (x == 0)
  {
    float a = ads.readADC_SingleEnded(ads_batPin) * ADC_convFactor * 0.001 / vbat_vdiv;
    return a;
  }
  else if (x == 1)
  {
    float a = ads.readADC_SingleEnded(ads_solarPin) * ADC_convFactor * 0.001 / solarVoltageDiv;
    return a;
  }
}

void startGPIO()
{
  pinMode(A0, INPUT);
}
float get_vmeasure_analog(const int x = 5, const int del = 20)
{
  float v_measure = 0;
  for (int i = 0; i < x; i++)
  {
    v_measure += (analogRead(A0));
    delay(del);
  }
  v_measure /= x;
  v_measure = v_measure / (float)ADC_BIT * vlogic / vbat_vdiv;
  return v_measure;
}

bool get_voltage_measures()
{
  if (vmeasure_type == 2)
  {
    startGPIO();
    vbat = get_vmeasure_analog();
    return 1;
  }
  else if (vmeasure_type == 1)
  {
    startADS();
    vbat = get_vmeasure_ADS(0);
    vsolarpannel = get_vmeasure_ADS(1);
    return 1;
  }
  else
  {
    return 0;
  }
}