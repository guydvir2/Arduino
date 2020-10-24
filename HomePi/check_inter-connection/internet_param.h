#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

#define JSON_SIZE_IOT 400
bool readfile_ok = false;

StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
extern myIOT iot;

void update_vars()
{
}
void startRead_parameters()
{
  String myIOT_defs = "{\"useSerial\":false,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":false, \"deviceTopic\" : \"internetmonitor\",\
                        \"groupTopic\" : \"\",\"prefixTopic\" : \"myHome\",\"extTopic\":\"myHome/Telegram\",\"debug_level\":2,\"noNetwork_reset\":1}";

  if (iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON))
  {
    readfile_ok = true;
  }
  // serializeJsonPretty(paramJSON, Serial);
  // update_vars(sketchJSON);
}
void endRead_parameters()
{
  if (!readfile_ok)
  {
    iot.pub_log("Error read Parameters from file. Defaults values loaded.");
  }
  else
  paramJSON.clear();
}