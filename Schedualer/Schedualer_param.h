#include <myIOT2.h>

extern myIOT2 iot;

char *sketch_paramfile = "/sketch_param.json";

void update_vars(JsonDocument &DOC)
{
  numSW = DOC["numSW"];
  PWM_res = DOC["PWM_res"];
  sketch_JSON_Psize = DOC["sketch_JSON_Psize"];

  for (int i = 0; i < numSW; i++)
  {
    useInputs[i] = DOC["useInputs"][i];
    inputPin[i] = DOC["inputPin"][i];
    inputPressed[i] = DOC["inputPressed"][i];

    OnatBoot[i] = DOC["OnatBoot"][i];
    output_ON[i] = DOC["output_ON"][i];
    outputPin[i] = DOC["outputPin"][i];

    defPWM[i] = DOC["defPWM"][i];
    limitPWM[i] = DOC["limitPWM"][i];
    outputPWM[i] = DOC["outputPWM"][i];

    indic_ON[i] = DOC["indic_ON"][i];
    indicPin[i] = DOC["indicPin"][i];
    useIndicLED[i] = DOC["useIndicLED"][i];

    maxTimeout[i] = DOC["maxTimeout"][i];

    defaultTimeout[i] = DOC["defaultTimeout"][i];

    // TOsw[i]->trigType = DOC["trigType"][i];
    // TOsw[i]->max_pCount = DOC["max_pCount"][i];
    strcpy(sw_names[i], DOC["sw_names"][i].as<const char *>());
  }
}
void read_flashParameter()
{
  const int JSON_SIZE_SKETCH = 1250;
  StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

  if (iot.extract_JSON_from_flash(sketch_paramfile, sketchJSON))
  {
    update_vars(sketchJSON);
    // serializeJson(sketchJSON, Serial);
    // Serial.flush();
  }
  else
  {
    Serial.println("File read Error");
  }
  sketchJSON.clear();
}
