#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>
#include <myIOT2.h>

#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 400

bool readfile_ok = false;
StaticJsonDocument<JSON_SIZE_IOT> paramJSON;
StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;
char *sketch_paramfile = "/sketch_param.json";

extern myIOT2 iot;
extern int SleepDuration;
extern int forceWake;
extern int ads_batPin;
extern int ads_solarPin;
extern int vmeasure_type;
extern float MAX_SOLAR;
extern float ADC_convFactor;
extern float solarVoltageDiv;
extern const char *boardType;

void update_vars(JsonDocument &DOC)
{
  SleepDuration = DOC["sleep_duration"];
  forceWake = DOC["forceWake"];
  vmeasure_type = DOC["vmeasure_type"];
  ads_batPin = DOC["ads_batPin"];
  ads_solarPin = DOC["ads_solarPin"];
  MAX_SOLAR = DOC["solarPanel_v"];
  ADC_convFactor = DOC["ADC_convFactor"];
  solarVoltageDiv = DOC["solarVoltageDiv"];
  vbat_vdiv = DOC["vbat_vdiv"];
  const char *boardType = DOC["boardType"];
}
void startRead_parameters()
{
  String sketch_defs = "{\"sleep_duration\":1,\"forceWake\":60,\"vmeasure_type\":0,\"ads_batPin\":1,\
                        \"ads_solarPin\":2,\"solarPanel_v\":6.0,\"ADC_convFactor\":0.1875,\"solarVoltageDiv\":0.66,\"vbat_vdiv\":5.0,\
                        \"boardType\":\"esp8266\"}";

  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useextTopic\":true,\
                        \"useFailNTP\" : true,\"useDebugLog\" : false,\"useNetworkReset\":false, \"deviceTopic\" : \"esp8266\",\
                        \"groupTopic\" : \"SolarPower\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

  bool a = iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON);
  bool b = iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, paramJSON);
  readfile_ok = a && b;
  update_vars(sketchJSON);

  // serializeJsonPretty(sketchJSON, Serial);
  // serializeJsonPretty(paramJSON, Serial);
  // Serial.flush();
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
