#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

char *jsom_param_file = "/empty_IOTpar.json";
bool readfile_ok = false;

StaticJsonDocument<300> paramJSON;
String json_def_value = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\"useFailNTP\" : true,\"useDebugLog\" : true,\"deviceTopic\" : \"devTopic\",\"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0}";

void update_vars()
{
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
  Serial.flush();
  // update_vars();
}
void free_paramJSON()
{
  paramJSON.clear();
} 