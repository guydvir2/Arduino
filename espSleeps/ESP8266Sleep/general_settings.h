/* 
All Parameters in this file will be changed while reading parameters from JSON file.
*/
const float vlogic = 3.3;
const int ADC_BIT = 1023;
const float MAX_BAT = 4.15;
const float MIN_BAT = 2.8;
float MAX_SOLAR = 6.0;

// ~~~~~~~~~~~~~~ ADS 1115 ~~~~~~~~~~~~~~~
int ads_batPin = 0;   /* on ADC board*/
int ads_solarPin = 1; /* on ADC board*/
float ADC_convFactor = 0.1875;

// ~~~~~~~~~~~~~~ voltageDivider ~~~~~~~~~~~~~~~
int vmeasure_type = 0; /* 0 - none, 1- ADS, 2-io */
float vbat_vdiv = 5.0;
float solarVoltageDiv = 0.66; /*in case that measuing SolarPanel Voltage*/

// ~~~~~~~~~~ Sleep&Wake ~~~~~~~~~~~~
const char *boardType = "esp8266_builtin_battery";
int SleepDuration = 5; /* Minutes */
int forceWake = 30;    /* Seconds */

// ~~~~~~~~~~~~~~ voltageDivider ~~~~~~~~~~~~~~~

