#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

char *jsom_param_file = "/sw_IOTpar.json";
bool readfile_ok = false;

StaticJsonDocument<1300> paramJSON;
String json_def_value = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\"useFailNTP\" : true,\"useDebugLog\" : true,\"useDisplay\":false,\"deviceTopic\" : \"devTopic\",\"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"useOnatBoot\" : false,\"usequickBoot\" : false,\"usedailyTO\" : true,\"useSafteyOff\" : true,\"useEEPROM_resetCounter\" : false,\"usePWM\" : true,\"useInput\" : false,\"useExtTrig\" : false,\"momentryButtorn\" : true,\"numSW\" : 1,\"debug_level\":2,\"safetyOffDuration\" : 60,\"inputPin\" : [0, 2],\"outputPin\":[12,5],\"extTrigPin\" : 5,\"hReboots\" : [1,2],\"start_dTO\" : [[19,0, 0],[20,30,0]],\"end_dTO\" : [[23,30,0],[22,0,0]],\"timeOUTS\" : [120,120],\"SW_Names\" : [\"LED1\",\"LED2\"],\"defPWM\":0.7}";

extern int START_dTO[numSW][3];  // = {{19, 0, 0}, {21, 30, 0}};
extern int END_dTO[numSW][3];    // = {{23, 30, 0}, {22, 0, 0}};
extern int TimeOUT[numSW];       // = {10, 120}; // minutes
extern int outputPin[numSW];     // = {12, 5}; // D3 for most PWM boards
extern int inputPin[numSW];      // = {0, 2};	 // input is not for extTrig
extern int extTrigPin;       // = 5;
extern int hRebbots[numSW];      // = {1, 2};
extern char SW_Names[numSW][30]; // = {"LEDstrip", "Ledstrip"};
extern bool usePWM;
extern bool useExtTrig;

/*
 ~~~~~ SONOFF HARDWARE ~~~~~
 #define RELAY1 12
 #define RELAY2 5
 #define INPUT1 0  // 0 for onBoard Button
 #define INPUT2 14 // 14 for extButton
 #define indic_LEDpin 13
 */

// ~~~~~~~~~~~~~~~~~~~~~~~

void update_vars()
{
  for (int a = 0; a < numSW; a++)
  {
    inputPin[a] = paramJSON["inputPin"][a];
    outputPin[a] = paramJSON["outputPin"][a];
    hRebbots[a] = paramJSON["hReboots"][a];
    TimeOUT[a] = paramJSON["timeOUTS"][a];
    strcpy(SW_Names[a], paramJSON["SW_Names"][a].as<const char*>());

    for (int i = 0; i < 3; i++)
    {
      START_dTO[a][i] = paramJSON["start_dTO"][a][i];
      END_dTO[a][i] = paramJSON["end_dTO"][a][i];
    }
  }
  extTrigPin = paramJSON["extTrigPin"];
  useExtTrig = paramJSON["useExtTrig"];
  usePWM = paramJSON["usePWM"];
}
void read_parameters_from_file()
{
  myJSON param_of_flash(jsom_param_file);

  if (param_of_flash.file_exists())
  {
    if (param_of_flash.readJSON_file(paramJSON))
    {
      readfile_ok = true;
    }
  }
  else
  {
    deserializeJson(paramJSON, json_def_value);
  }
  serializeJsonPretty(paramJSON, Serial);
  update_vars();
}
void free_paramJSON()
{
  paramJSON.clear();
}