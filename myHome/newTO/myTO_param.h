#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

#define JSON_SIZE_IOT 500
#define JSON_SIZE_SKETCH 1000

char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;

StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;
StaticJsonDocument<JSON_SIZE_IOT> paramJSON;

void update_vars(JsonDocument &DOC)
{
  numSW = DOC["numSW"];
  PWM_res = DOC["PWM_res"];
  for (int i = 0; i < numSW; i++)
  {
    inputPin[i] = DOC["inputPin"][i];
    outputPin[i] = DOC["outputPin"][i];
    inputPressed[i] = DOC["inputPressed"][i];
    output_ON[i] = DOC["output_ON"][i];
    OnatBoot[i] = DOC["OnatBoot"][i];
    defPWM[i] = DOC["defPWM"][i];
    limitPWM[i] = DOC["limitPWM"][i];


    TOsw[i]->useInput =DOC["useInput"][i];
    TOsw[i]->maxON_minutes = DOC["maxON_minutes"][i];
    TOsw[i]->def_TO_minutes = DOC["def_TO_minutes"][i];
    TOsw[i]->trigType = DOC["trigType"][i];
    TOsw[i]->totPWMsteps = DOC["totPWMsteps"][i];
    strcpy(sw_names[i], DOC["sw_names"][i].as<const char *>());
  }
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  String sketch_defs = "{\"useInput\":[true,true],\"trigType\":[3,3], \"def_TO_minutes\":[120,100],\"maxON_minutes\":[720,360],\
                        \"inputPressed\":[true, false],\"inputPin\":[5,6], \"outputPin\":[4,3],\"output_ON\":[true,true],\
                        \"PWM_res\":1023,\"totPWMsteps\":[3,3], \"defPWM\":[2,2], \"limitPWM\":[80,80],\"OnatBoot\":[true,false],\
                        \"numSW\":1, \"sw_names\":[\"sw0\",\"sw1\"]}";

  bool a = iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON);
  bool b = iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON);
  readfile_ok = b ;//&& a;
  // serializeJsonPretty(sketchJSON, Serial);
  // serializeJsonPretty(paramJSON, Serial);
  // Serial.flush();
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
