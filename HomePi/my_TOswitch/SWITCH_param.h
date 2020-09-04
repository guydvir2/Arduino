#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>


#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 1300
char *myIOT_paramfile = "/myIOT_param.json";
char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;

StaticJsonDocument<JSON_SIZE_IOT> paramJSON;

static const int maxSW = 2;

extern int numSW;
extern int START_dTO[maxSW][3];  // = {{19, 0, 0}, {21, 30, 0}};
extern int END_dTO[maxSW][3];    // = {{23, 30, 0}, {22, 0, 0}};
extern int TimeOUT[maxSW];       // = {10, 120}; // minutes
extern int outputPin[maxSW];     // = {12, 5}; // D3 for most PWM boards
extern int inputPin[maxSW];      // = {0, 2};	 // input is not for extTrig
extern int extTrigPin;           // = 5;
extern int hRebbots[maxSW];      // = {1, 2};
extern char SW_Names[maxSW][30]; // = {"LEDstrip", "Ledstrip"};
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

void update_vars(JsonDocument &DOC)
{
  numSW = DOC["numSW"];
  for (int a = 0; a < numSW; a++)
  {
    inputPin[a] = DOC["inputPin"][a];
    outputPin[a] = DOC["outputPin"][a];
    hRebbots[a] = DOC["hReboots"][a];
    TimeOUT[a] = DOC["timeOUTS"][a];
    strcpy(SW_Names[a], DOC["SW_Names"][a].as<const char *>());

    for (int i = 0; i < 3; i++)
    {
      START_dTO[a][i] = DOC["start_dTO"][a][i];
      END_dTO[a][i] = DOC["end_dTO"][a][i];
    }
  }
  extTrigPin = DOC["extTrigPin"];
  useExtTrig = DOC["useExtTrig"];
  usePWM = DOC["usePWM"];
}

// ±±±±±±±±±±±±±±±

void read_flash_parameters(char *filename, String &defs, JsonDocument &DOC)
{
  myJSON param_on_flash(filename, true, JSON_SIZE_IOT);

  if (param_on_flash.file_exists())
  {
    if (param_on_flash.readJSON_file(DOC))
    {
      readfile_ok = true;
    }
  }
  else
  {
    Serial.printf("\nfile %s read NOT-OK", filename);
    deserializeJson(DOC, defs);
  }
  serializeJsonPretty(DOC, Serial);
  Serial.flush();
}
void read_parameters_from_file()
{
  StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

  String sketch_defs = "{\"useDisplay\":false,\"useOnatBoot\" : false,\"usequickBoot\" : false,\"usedailyTO\" : true,\
                        \"useSafteyOff\" : true,\"useEEPROM_resetCounter\" : false,\"usePWM\" : true,\"useInput\" : false,\
                        \"useExtTrig\" : false,\"momentryButtorn\" : true,\"numSW\" : 1,\"safetyOffDuration\" : 60,\
                        \"inputPin\" : [0, 2],\"outputPin\":[12,5],\"extTrigPin\" : 5,\"hReboots\" : [1,2],\
                        \"start_dTO\" : [[19,0, 0],[20,30,0]],\"end_dTO\" : [[23,30,0],[22,0,0]],\"timeOUTS\" : [120,120],\
                        \"SW_Names\" : [\"LED1\",\"LED2\"],\"defPWM\":0.7}";

  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \" groupTopic\" : \"group\",\" prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  read_flash_parameters(myIOT_paramfile, myIOT_defs, paramJSON);
  read_flash_parameters(sketch_paramfile, sketch_defs, sketchJSON);
  update_vars(sketchJSON);
}
void free_paramJSON()
{
  paramJSON.clear();
}