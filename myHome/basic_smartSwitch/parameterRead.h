bool bootProcess_OK = false;

extern void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t pwm_pwr,
                         uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot);

/* Topics */
void assign_filenames()
{
  iot.parameter_filenames[0] = "/myIOT2_topics.json";
  iot.parameter_filenames[1] = "/sketch_param.json";
  Serial.println(F("~ Assign filenames - OK"));
}
bool _storeTopics_in_array(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0) /* update local Topic array */
{
  uint8_t i = 0;
  JsonArray array = DOC[topic];

  if (!array.isNull())
  {
    for (const char *topic : array)
    {
      strlcpy(ch_array[i], topic, MAX_TOPIC_SIZE);
      dest_array[i + shift] = ch_array[i];
      i++;
    }
    return 1;
  }
  else
  {
    return 0;
  }
}
void update_hardCoded_topics()
{
  iot.topics_gen_pub[0] = "myHome/Messages";
  iot.topics_gen_pub[1] = "myHome/log";
  iot.topics_gen_pub[2] = "myHome/debug";

  iot.topics_pub[0] = "myHome/smartSW";
  iot.topics_pub[0] = "myHome/smartSW/Avail";
  iot.topics_pub[1] = "myHome/smartSW/State";

  iot.topics_sub[0] = "myHome/smartSW";
  iot.topics_sub[1] = "myHome/All";
}
bool readTopics_inFlash()
{
  StaticJsonDocument<1500> DOC;
#if MAN_MODE == true
  Serial.println(F("~ Topics database - local"));
  DeserializationError error0 = deserializeJson(DOC, topics);
  if (!error0)
  {
#else
  if (iot.readJson_inFlash(DOC, iot.parameter_filenames[0])) /* extract topics from flash */
  {
    Serial.println(F("~ Topics data base - flash"));
#endif
    if (_storeTopics_in_array(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics") &&
        _storeTopics_in_array(DOC, topics_pub, iot.topics_pub, "pub_topics") &&
        _storeTopics_in_array(DOC, topics_sub, iot.topics_sub, "sub_topics"))
    {
      Serial.println(F("~ Topics build - OK "));
      return 1;
    }
    else
    {
      Serial.println(F("~ Topics build - Failed "));
      return 0;
    }
  }
  else
  {
    Serial.println(F("~ Topics build from Flash - Failed "));
    return 0;
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~

bool get_entities_parameters()
{
  StaticJsonDocument<1200> DOC;
  uint8_t numS = 1;
  uint8_t mcuType = 0;
  bool useclkdown = false;

  const char *SWname[MAX_SW]{};
  uint8_t butType[MAX_SW] = {0, 0};
  uint8_t pwm_pwr[MAX_SW] = {0, 0};
  uint8_t inputPins[MAX_SW] = {255, 255};
  uint8_t outputPins[MAX_SW] = {254, 254}; /* 255 defines a virtual output */
  uint8_t indicPins[MAX_SW] = {255, 255};
  bool indic_on[MAX_SW] = {LOW, LOW};
  bool onatboot[MAX_SW] = {false, false};
  bool virtOutput[MAX_SW] = {false, false};
  int timeout_duration[MAX_SW] = {0, 0};

  if (iot.readJson_inFlash(DOC, iot.parameter_filenames[1]))
  {
    numS = DOC["numSW"];
    mcuType = DOC["MCUtype"];
    useclkdown = DOC["useLockdown"];
    set_Pins_to_MCU(mcuType, inputPins, outputPins, indicPins);

    for (uint8_t x = 0; x < numS; x++)
    {
      onatboot[x] = DOC["ONatBoot"][x];
      butType[x] = DOC["SWtype"][x];
      SWname[x] = DOC["name"][x].as<const char *>();
      timeout_duration[x] = DOC["Timeout"][x];
      indic_on[x] = DOC["indic_on"][x];
      pwm_pwr[x] = DOC["PWM_pwr"][x];
      virtOutput[x] = DOC["virtOutput"][x]; /* overides any pin / pwm def */

      smartSW_defs(x, SWname[x], butType[x], virtOutput[x] ? 255 : outputPins[x], pwm_pwr[x], inputPins[x], indicPins[x],
                   useclkdown, timeout_duration[x], indic_on[x], onatboot[x]);
    }
  }
  else
  {
    return 0;
  }
  DOC.clear();

  return 1;
}
void getStored_parameters()
{
  Serial.println(F("\n±±±±±±±±±±±± Start Reading Parameters ±±±±±±±±±±±±"));
  Serial.flush();
  assign_filenames();
  set_IOT2_Parameters();
  if (get_entities_parameters() && readTopics_inFlash())
  {
    bootProcess_OK = true;
    Serial.println(F("\n\n±±±±±±±±±±±± Reading Parameters -OK  ±±±±±±±±±±±±"));
    Serial.flush();
  }
  else
  {
    Serial.println(F("\n±±±±±±±±±±±± Reading Parameters -Failed  ±±±±±±±±±±±±"));
    Serial.flush();
    update_hardCoded_topics();
    bootProcess_OK = false;
  }
}