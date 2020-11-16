/* 
All Parameters in this file will be changed while reading parameters from JSON file.
*/

#define isESP32 true

float MAX_BAT = 4.1;
float MAX_SOLAR = 6.0;

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
bool ADS_vmeasure = false; /* verify one one to be true or false */
int ads_batPin = 0;   /* on ADC board*/
int ads_solarPin = 1; /* on ADC board*/
float ADC_convFactor = 0.1875;
float solarVoltageDiv = 0.66; /*in case that measuing SolarPanel Voltage*/

// ~~~~~~~~~~~~~~ voltageDivider ~~~~~~~~~~~~~~~
bool analog_vmeasure = true; /* and this one too*/
float vbat_vdiv = 5.0;

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
const char *boardType = "esp8266_builtin_battery";
int SleepDuration = 5; /* Minutes */
int forceWake = 30;    /* Seconds */