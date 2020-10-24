#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 200

char *sketch_paramfile = "/sketch_param.json";
bool readfile_ok = false;


StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

char paramA[20];
int paramB = 0;
extern myIOT iot;

void update_vars(JsonDocument &DOC)
{
  strcpy(paramA, DOC["paramA"]);
  paramB = DOC["paramB"];
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  String sketch_defs = "{\"paramA\":\"BBB\",\"paramB\":5555}";


  if (iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON) && iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON))
  {
    readfile_ok = true;
  }
  // update_var/s(sketchJSON);
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







// void read_flash_parameters(char *filename, String &defs, JsonDocument &DOC)
// {
//   myJSON param_on_flash(filename, true, JSON_SIZE_IOT);

//   if (param_on_flash.file_exists())
//   {
//     if (param_on_flash.readJSON_file(DOC))
//     {
//       readfile_ok = true;
//     }
//   }
//   else
//   {
//     Serial.printf("\nfile %s read NOT-OK", filename);
//     deserializeJson(DOC, defs);
//   }
//   serializeJsonPretty(DOC, Serial);
//   Serial.flush();
// }
// void read_parameters_from_file()
// {
  

//   String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
//                         \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
//                         \" groupTopic\" : \"group\",\" prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

//   String sketch_defs = "{\"paramA\":\"BBB\",\"paramB\":5555}";

//   read_flash_parameters(myIOT_paramfile, myIOT_defs, paramJSON);
//   read_flash_parameters(sketch_paramfile, sketch_defs, sketchJSON);
//   update_vars(sketchJSON);
// }
// void free_paramJSON()
// {
//   if (!readfile_ok)
//   {
//     iot.pub_log("Error read Parameters from file. Defaults values loaded.");
//   }
//   paramJSON.clear();
//   sketchJSON.clear();
// }
