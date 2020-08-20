String json_def_value = "{\"ext_inputs\":false,\"auto_relay_off\":false,\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\": false,\"useFailNTP\":true,\"useDebugLog\":true,\"deviceTopic\":\"devTopic\",\"groupTopic\":\"noGroup\",\"prefixTopic\":\"myHome\",\"inputUpPin\":4,\"inputDownPin\":5,\"outputUpPin\":14,\"outputDownPin\":12,\"inputUpExtPin\":0,\"inputDownExtPin\":2,\"auto_relay_off_timeout\":60}";

myJSON param_of_flash("/win_IOTpar.json");
StaticJsonDocument<600> paramJSON;

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

bool readfile_ok=false;

void update_vars()
{
  inputUpPin = paramJSON["inputUpPin"];
  inputDownPin = paramJSON["inputDownPin"];
  outputUpPin = paramJSON["outputUpPin"];
  outputDownPin = paramJSON["outputDownPin"];
  inputUpExtPin = paramJSON["inputUpExtPin"];
  inputDownExtPin = paramJSON["inputDownExtPin"];

  ext_inputs = paramJSON["ext_inputs"];
  auto_relay_off = paramJSON["auto_relay_off"];
  auto_relay_off_timeout = paramJSON["auto_relay_off_timeout"];
}
void read_parameters_from_file()
{
  if (param_of_flash.file_exists())
  {
    if(param_of_flash.readJSON_file(paramJSON)){
          Serial.println("saved_values");
          readfile_ok=true;
    }
  }
  else
  {
    deserializeJson(paramJSON, json_def_value);
    Serial.println("def_values");
  }
  serializeJsonPretty(paramJSON, Serial);
  // Serial.println(paramJSON.memoryUsage());
  update_vars();
}
