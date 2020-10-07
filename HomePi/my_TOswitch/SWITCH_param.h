#include <Arduino.h>
#include <myIOT.h>
#include <ArduinoJson.h>

#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 1300
char *sketch_paramfile = "/sketch_param.json";
static const int maxSW = 2;

bool readfile_ok = false;
StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

extern myIOT iot;
extern int numSW;
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
  extTrigPin = DOC["extTrigPin"];
  useExtTrig = DOC["useExtTrig"];
  usePWM = DOC["usePWM"];

  for (int a = 0; a < numSW; a++)
  {
    inputPin[a] = DOC["inputPin"][a];
    outputPin[a] = DOC["outputPin"][a];
    hRebbots[a] = DOC["hReboots"][a];
    strcpy(SW_Names[a], DOC["SW_Names"][a].as<const char *>());
  }
}

void startRead_parameters()
{
  String sketch_defs = "{\"useDisplay\":false,\"useOnatBoot\" : false,\"usequickBoot\" : false,\"usedailyTO\" : true,\
                        \"useSafteyOff\" : true,\"useResetKeeper\":false,\"usetimeOUT\":false,\"useEEPROM_resetCounter\" : false,\
                        \"usePWM\" : true,\"useInput\" : false,\"useExtTrig\" : false,\"momentryButtorn\" : true,\"inputState\":true,\
                        \"numSW\" : 1, \"safetyOffDuration\" : 60,\"inputPin\" : [0, 2],\"outputPin\":[12,5],\"extTrigPin\" : 5,\
                        \"hReboots\" : [1,2],\"start_dTO\" : [[19,0, 0],[20,30,0]],\"end_dTO\" : [[23,30,0],[22,0,0]],\
                        \"timeOUTS\" : [120,120],\"SW_Names\" : [\"LED1\",\"LED2\"],\"defPWM\":0.7}";

  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":false, \"deviceTopic\" : \"mySwitch\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  if (iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON) && iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON))
  {
    readfile_ok = true;
  }
  // serializeJsonPretty(sketchJSON, Serial);
  update_vars(sketchJSON);
}
void endRead_parameters()
{
  if (!readfile_ok)
  {
    iot.pub_log("Error read Parameters from file. Defaults values loaded.");
  }
  paramJSON.clear();
  sketchJSON.clear();
}