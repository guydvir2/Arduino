#include <Arduino.h>
#include <Adafruit_ADS1015.h>

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
bool VoltageMeasures = true;
int ads_batPin = 0;   /* on ADC board*/
int ads_solarPin = 1; /* on ADC board*/
float ADC_convFactor = 0.1875;
float solarVoltageDiv = 0.66; /*in case that measuing SolarPanel Voltage*/
float MAX_BAT = 4.1;
float MAX_SOLAR = 6.0;

float ADC_bat = 0;
float ADC_solarPanel = 0;

Adafruit_ADS1115 ads;

void startADS()
{
  if(VoltageMeasures){
  ads.begin();
  // ads.setGain(GAIN_TWOTHIRDS); // 0.1875mV (default)
  }
}
void measureADS()
{
  ADC_bat = ads.readADC_SingleEnded(ads_batPin) * ADC_convFactor * 0.001;
  ADC_solarPanel = ads.readADC_SingleEnded(ads_solarPin) * ADC_convFactor * 0.001 / solarVoltageDiv;
}
