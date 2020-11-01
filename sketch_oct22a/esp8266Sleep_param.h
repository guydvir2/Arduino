#include <Arduino.h>
#include <myIOT.h>
#include <ArduinoJson.h>

#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 400
char *sketch_paramfile = "/sketch_param.json";

bool readfile_ok = false;
StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

extern myIOT iot;
extern int SleepDuration;
extern int forceWake;
extern int ads_batPin;
extern int ads_solarPin;
extern bool VoltageMeasures;
extern float ADC_solarPanel;
extern float MAX_SOLAR;
extern float ADC_convFactor;
extern float solarVoltageDiv;

void update_vars(JsonDocument &DOC)
{
  SleepDuration = DOC["sleep_duration"];
  forceWake = DOC["forceWake"];
  VoltageMeasures = DOC["measure_voltage"];
  ads_batPin = DOC["ads_batPin"];
  ads_solarPin = DOC["ads_solarPin"];
  MAX_SOLAR = DOC["solarPanel_v"];
  ADC_convFactor = DOC["ADC_convFactor"];
  solarVoltageDiv = DOC["solarVoltageDiv"];
}
void startRead_parameters()
{
  String sketch_defs = "{\"sleep_duration\":5,\"forceWake\":60,\"measure_voltage\":true,\"ads_batPin\":1,\
                        \"ads_solarPin\":2,\"solarPanel_v\":6.0,\"ADC_convFactor\":0.1875,\"solarVoltageDiv\":0.66,\
                        \"boardType\":\"esp8266\"}";

  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\
                        \"useFailNTP\" : true,\"useDebugLog\" : false,\"useNetworkReset\":false, \"deviceTopic\" : \"esp8266\",\
                        \"groupTopic\" : \"Solarpower\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";


  bool a = iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON);
  bool b = iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON);
  readfile_ok = a && b;

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