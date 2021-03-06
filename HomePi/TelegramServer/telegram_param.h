#include <Arduino.h>
#include <ArduinoJson.h>

#define JSON_SIZE_IOT 500
// #define JSON_SIZE_SKETCH 200

// char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;


StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
// StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

extern myIOT iot;

void update_vars(JsonDocument &DOC)
{
  // strcpy(paramA, DOC["paramA"]);
  // paramB = DOC["paramB"];
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"useextTopic\":true,\
                        \"extTopic\": \"extopic\",\"deviceTopic\" : \"devTopic\",\"groupTopic\" : \"group\",\
                        \"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":5}";

  if (iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON))
  {
    readfile_ok = true;
  }
  // update_vars(sketchJSON);
}
void endRead_parameters()
{
  if (!readfile_ok)
  {
    iot.pub_log("Error read Parameters from file. Defaults values loaded.");
  }
  paramJSON.clear();
}