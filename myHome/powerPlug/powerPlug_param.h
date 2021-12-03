#include <ArduinoJson.h>
#include <myJSON.h>

#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 1200

char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;

StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

extern myIOT2 iot;

void update_vars(JsonDocument &DOC)
{
  // strcpy(paramA, DOC["paramA"]);
  // paramB = DOC["paramB"];
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  String sketch_defs = "{\"useOnatBoot\" : false,\"usequickBoot\" : false,\"usedailyTO\" : true,\
                        \"useSafteyOff\" : true,\"useResetKeeper\":false,\"usetimeOUT\":false,\"useEEPROM_resetCounter\" : false,\
                        \"usePWM\" : true,\"useInput\" : false,\"useExtTrig\" : false,\"momentryButtorn\" : true,\"inputState\":true,\
                        \"numSW\" : 1, \"safetyOffDuration\" : 60,\"inputPin\" : [0, 2],\"outputPin\":[12,5],\"extTrigPin\" : 5,\
                        \"hReboots\" : [1,2],\"start_dTO\" : [[19,0, 0],[20,30,0]],\"end_dTO\" : [[23,30,0],[22,0,0]],\
                        \"timeOUTS\" : [120,120],\"SW_Names\" : [\"powerPlug\",\"LED2\"],\"defPWM\":0.7}";


  bool a = iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON);
  bool b = iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON);
  readfile_ok = a && b;

  // serializeJsonPretty(sketchJSON, Serial);
  // serializeJsonPretty(paramJSON, Serial);
  // Serial.flush();
  // update_vars(sketchJSON);

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
