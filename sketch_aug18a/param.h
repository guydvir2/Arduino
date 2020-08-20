String json_def_value = "{\"ext_inputs\":false,\"auto_relay_off\":false,\"useSerial\":false,\"useWDT\":true,\"useOTA\":true,\"useResetKeeper\": false,\"useFailNTP\":true,\"useDebugLog\":true,\"deviceTopic\":\"devTopic\",\"groupTopic\":\"noGroup\",\"prefixTopic\":\"myHome\",\"inputUpPin\":4,\"inputDownPin\":5,\"outputUpPin\":14,\"outputDownPin\":12,\"inputUpExtPin\":0,\"inputDownExtPin\":2,\"auto_relay_off_timeout\":60}";

myJSON param_of_flash("/parameters.json");
DynamicJsonDocument paramJSON(500);

void sayhi()
{
  Serial.println("HI");
}

void readfromfile()
{

  if (param_of_flash.FS_ok())
  {
    param_of_flash.readJSON_file(paramJSON);
    // serializeJsonPretty(paramJSON, Serial);
    }
  else
  {
    deserializeJson(paramJSON, json_def_value);
  }
}
