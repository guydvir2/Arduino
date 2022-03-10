#include <Arduino.h>
#include <myIOT2.h>

extern myIOT2 iot;
extern void init_timeOUT();
char *sketch_paramfile = "/sketch_param.json";
#define JSON_SIZE 1250
void update_vars(JsonDocument &DOC)
{
  numSW = DOC["numSW"];
  PWM_res = DOC["PWM_res"];
  init_timeOUT();

  for (int i = 0; i < numSW; i++)
  {
    inputPin[i] = DOC["inputPin"][i];
    outputPin[i] = DOC["outputPin"][i];
    inputPressed[i] = DOC["inputPressed"][i];
    output_ON[i] = DOC["output_ON"][i];
    OnatBoot[i] = DOC["OnatBoot"][i];
    defPWM[i] = DOC["defPWM"][i];
    limitPWM[i] = DOC["limitPWM"][i];
    outputPWM[i] = DOC["outputPWM"][i];
    useIndicLED[i] = DOC["useIndicLED"][i];
    indic_ON[i] = DOC["indic_ON"][i];
    indicPin[i] = DOC["indicPin"][i];

    TOsw[i]->useInput = DOC["useInput"][i];
    TOsw[i]->maxON_minutes = DOC["maxON_minutes"][i];
    TOsw[i]->def_TO_minutes = DOC["def_TO_minutes"][i];
    TOsw[i]->trigType = DOC["trigType"][i];
    TOsw[i]->max_pCount = DOC["max_pCount"][i];
    strcpy(sw_names[i], DOC["sw_names"][i].as<const char *>());
  }
}
void read_flashParameter()
{
  DynamicJsonDocument sketchJSON(JSON_SIZE);
  String sketch_defs = "{\"useInput\":[true,true],\"trigType\":[3,3], \"def_TO_minutes\":[120,100],\"maxON_minutes\":[720,360],\
                        \"inputPressed\":[true, false],\"inputPin\":[5,6], \"outputPin\":[4,3],\"output_ON\":[true,true],\
                        \"PWM_res\":1023,\"max_pCount\":[3,3], \"defPWM\":[2,2], \"limitPWM\":[80,80],\"OnatBoot\":[true,false],\
                        \"numSW\":1, \"sw_names\":[\"sw0\",\"sw1\"],\"useIndicLED\":[false, false],\"indic_ON\":[false,false],\
                        \"indicPin\":[12,2]}";

  bool a = iot.read_fPars(sketch_paramfile, sketchJSON, sketch_defs); /* Read sketch defs */
  // serializeJson(sketchJSON, Serial);
  update_vars(sketchJSON);
  sketchJSON.clear();
}
