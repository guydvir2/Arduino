#include <ArduinoJson.h>
#include <myJSON.h>
#include <myTimeoutSwitch.h>


#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 1200

char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;

StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

extern myIOT2 iot;
extern timeOUTSwitch TOswitch;

void update_vars(JsonDocument &DOC)
{
  OnatBoot = DOC["OnatBoot"];
  TOswitch.maxON_minutes = DOC["maxON_minutes"];
  TOswitch.def_TO_minutes = DOC["def_TO_minutes"];
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  String sketch_defs = "{\"def_TO_minutes\":120,\"maxON_minutes\":300,\"OnatBoot\":true}";

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
