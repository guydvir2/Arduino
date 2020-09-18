#include <Arduino.h>
#include <myIOT.h>
#include <ArduinoJson.h>

#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 300
char *myIOT_paramfile = "/myIOT_param.json";
char *sketch_paramfile = "/sketch_param.json";

bool readfile_ok = false;
StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

extern myIOT iot;
//~~~~Internal Switch ~~~~~~
extern int inputUpPin;    // = D2;   // main is D2 // D3 only for saloonSingle
extern int inputDownPin;  // = D1; // main is D1 // D3 only for laundryRoom
extern int outputUpPin;   // = 14;
extern int outputDownPin; // = 12;
//~~~~External Input ~~~~~~~~~
extern int inputUpExtPin;   // = 0;
extern int inputDownExtPin; // = 2;

extern bool ext_inputs;
extern bool auto_relay_off;
extern int auto_relay_off_timeout;
//############################

void update_vars(JsonDocument &DOC)
{
  inputUpPin = DOC["inputUpPin"];
  inputDownPin = DOC["inputDownPin"];
  outputUpPin = DOC["outputUpPin"];
  outputDownPin = DOC["outputDownPin"];
  inputUpExtPin = DOC["inputUpExtPin"];
  inputDownExtPin = DOC["inputDownExtPin"];

  ext_inputs = DOC["ext_inputs"];
  auto_relay_off = DOC["auto_relay_off"];
  auto_relay_off_timeout = DOC["auto_relay_off_timeout"];
}
void startRead_parameters()
{
  String sketch_defs = "{\"ext_inputs\":false,\"auto_relay_off\":false,\"inputUpPin\":4,\"inputDownPin\":5,\
                        \"outputUpPin\":14,\"outputDownPin\":12,\"inputUpExtPin\":0,\"inputDownExtPin\":2,\"auto_relay_off_timeout\":60}";

  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":false, \"deviceTopic\" : \"myWindow\",\
                        \"groupTopic\" : \"Windows\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  if (iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON) && iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON))
  {
    readfile_ok = true;
  }
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
