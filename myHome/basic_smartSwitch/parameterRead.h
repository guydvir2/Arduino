bool bootProcess_OK = false;

extern void smartSW_defs(uint8_t id, const char *SWname, uint8_t butType, uint8_t output_pin, uint8_t pwm_pwr,
                         uint8_t input_pin, uint8_t indic_pin, bool uselckd, int timeout, bool indic_on, bool onatboot);
const char *MCUtypeIO[] = {"/io_fail.json", "/io_SONOFF_S26.json", "/io_SONOFF_mini.json", "/io_MCU.json", "/io_ESP01.json"};

void assign_filenames()
{
  iot.parameter_filenames[0] = "/myIOT_param.json";
  iot.parameter_filenames[1] = MCUtypeIO[0];
  iot.parameter_filenames[2] = "/myIOT2_topics.json";
  iot.parameter_filenames[3] = "/sketch_param.json";
  Serial.println(F("~ builded OK"));
}
void _storeTopics_in_array(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0) /* update local Topic array */
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
  if (iot.readJson_inFlash(DOC, iot.parameter_filenames[2])) /* extract topics from flash */
  {
    Serial.println(F("~ Topics data base - flash"));
#endif
    _storeTopics_in_array(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    _storeTopics_in_array(DOC, topics_pub, iot.topics_pub, "pub_topics");
    _storeTopics_in_array(DOC, topics_sub, iot.topics_sub, "sub_topics");

    Serial.println(F("~ Topics build - OK "));
    return 1;
  }
  else
  {
    Serial.println(F("~ Topics build from Flash - Failed "));
    return 0;
  }
}
bool get_IOT2_parameters()
{
#if MAN_MODE
  iot.useSerial = true;
  iot.useFlashP = false;
  iot.noNetwork_reset = 2;
  iot.ignore_boot_msg = false;
  Serial.println(F("~ IOT2 parameters - local"));
  return 1;
#else
  StaticJsonDocument<250> DOC;
  if (iot.readFlashParameters(DOC, iot.parameter_filenames[0]))
  {
    Serial.println("~ IOT2 parameters read - OK");
    return 1;
  }
  else
  {
    Serial.println("~ IOT2 parameters read - Failed. Using defaults.");
    return 0;
  }
#endif
}
bool get_entities_parameters()
{
  StaticJsonDocument<1200> DOC;
  const char *SWname{};
  uint8_t numS{};
  uint8_t mcuType{};
  uint8_t butType{};
  uint8_t input_pin{};
  uint8_t indic_pin{};
  uint8_t output_pin{};
  uint8_t pwm_pwr{};
  int timeout_duration{};
  bool useclkdown = false;
  bool indic_on = false;
  bool onatboot = false;

  uint8_t x = 0;

  while (x < MAX_SW)
  {
    /* Part 1: Read Sketch paramters from flash, and update Sketch */
    if (iot.readJson_inFlash(DOC, iot.parameter_filenames[3]))
    {
      numS = DOC["numSW"];
      mcuType = DOC["MCUtype"];
      useclkdown = DOC["useLockdown"];

      onatboot = DOC["ONatBoot"][x];
      butType = DOC["SWtype"][x];
      SWname = DOC["name"][x].as<const char *>();
      timeout_duration = DOC["Timeout"][x];
      iot.parameter_filenames[1] = MCUtypeIO[mcuType]; /* update filename of MCU type */
    }
    else
    {
      return 0;
    }
    DOC.clear();

    /* Part 2: Read MCU IOs paramters from flash, and update Sketch */
    if (iot.readJson_inFlash(DOC, iot.parameter_filenames[1]))
    {
      input_pin = DOC["inputPin"][x].as<uint8_t>();
      output_pin = DOC["relayPin"][x].as<uint8_t>();
      indic_pin = DOC["indicLED"][x] | 255;
      indic_on = DOC["indic_on"][x] | false;
      pwm_pwr = DOC["PWM_pwr"][x] | 0;
    }
    else
    {
      return 0;
    }
    smartSW_defs(x, SWname, butType, output_pin, pwm_pwr, input_pin, indic_pin, useclkdown, timeout_duration, indic_on, onatboot);
    DOC.clear();

    if (numS == 1)
    {
      x = MAX_SW;
    }
    else
    {
      x++;
    }
  }
  return 1;
}
void getStored_parameters()
{
  Serial.println(F("\n±±±±±±±±±±±± Start Reading Parameters n±±±±±±±±±±±±"));
  Serial.flush();
  assign_filenames();
  if (get_IOT2_parameters() && get_entities_parameters() && readTopics_inFlash())
  {
    bootProcess_OK = true;
    Serial.println(F("\n\n±±±±±±±±±±±± Reading Parameters -OK  n±±±±±±±±±±±±"));
    Serial.flush();
  }
  else
  {
    Serial.println(F("\n±±±±±±±±±±±± Reading Parameters -Failed  n±±±±±±±±±±±±"));
    Serial.flush();
    get_IOT2_parameters();
    update_hardCoded_topics();
    bootProcess_OK = false;
  }
}