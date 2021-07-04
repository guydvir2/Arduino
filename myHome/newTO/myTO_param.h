#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

#define JSON_SIZE_IOT 500
#define JSON_SIZE_SKETCH 500

char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;

StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;
StaticJsonDocument<JSON_SIZE_IOT> paramJSON;

extern myIOT2 iot;

void update_vars(JsonDocument &DOC)
{
  timeoutSW_0.useInput = DOC["useInput"];
  inputPressed = DOC["inputPressed"];
  timeoutSW_0.maxON_minutes = DOC["maxON_minutes"];
  timeoutSW_0.def_TO_minutes = DOC["def_TO_minutes"];
  timeoutSW_0.trigType = DOC["trigType"];
  timeoutSW_0.totPWMsteps = DOC["totPWMsteps"];

  output_ON = DOC["output_ON"];
  OnatBoot = DOC["OnatBoot"];
  inputPin = DOC["inputPin"];
  outputPin = DOC["outputPin"];
  defPWM = DOC["defPWM"];
  limitPWM = DOC["limitPWM"];
  PWM_res = DOC["PWM_res"];
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  String sketch_defs = "{\"useInput\":\"true\",\"trigType\":3, \"def_TO_minutes\":120,\"maxON_minutes\":720,\"inputPressed\":\"true\",\
                        \"inputPin\":5, \"outputPin\":4,\"output_ON\":\"true\",\"PWM_res\":1023,\"totPWMsteps\":3, \"defPWM\":2,\
                        \"limitPWM\":80,\"OnatBoot\":\"true\"}";

  bool a = iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON);
  bool b = iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON);
  readfile_ok = a && b;
  // serializeJsonPretty(sketchJSON, Serial);
  // serializeJsonPretty(paramJSON, Serial);
  // Serial.flush();

#if USE_SIMPLE_IOT == 0
  update_vars(sketchJSON);
#endif
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
