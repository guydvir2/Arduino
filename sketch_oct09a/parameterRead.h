extern bool getPins_manual(JsonDocument &DOC);
extern bool bootProcess_OK;

char parameterFiles[4][30];

void build_path_directory(uint8_t i = 0)
{
  const char *dirs[] = {"Fail", "Cont_A", "Cont_B", "Cont_C", "Cont_D", "Cont_test", "SONOFF_P"}; /* All directories have to be on MCU's flash */
  const char *FileNames_common[2] = {"myIOT_param.json", "HW.json"};
  const char *FileNames_dedicated[2] = {"myIOT2_topics.json", "sketch_param.json"};

  Serial.println(F("~ Build filenames:"));

  for (uint8_t x = 0; x < 4; x++)
  {
    if (x < 2)
    {
      sprintf(parameterFiles[x], "/%s", FileNames_common[x]);
    }
    else
    {
      sprintf(parameterFiles[x], "/%s/%s", dirs[i], FileNames_dedicated[x - 2]);
    }
    iot.parameter_filenames[x] = parameterFiles[x];
    Serial.println(iot.parameter_filenames[x]);
  }
  Serial.println(F("~ builded OK"));
}
void _updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0) /* update local Topic array */
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

bool readTopics()
{
  StaticJsonDocument<1500> DOC;
#if MAN_MODE == true
  Serial.println(F("~ Topics data-base - local"));
  DeserializationError error0 = deserializeJson(DOC, topics);

  if (!error0)
  {
#else
  if (iot.readJson_inFlash(DOC,iot.parameter_filenames[2])) /* extract topics from flash */
  {
    Serial.println(F("~ entities data-base - flash"));
#endif
    _updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    _updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
    _updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

    uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));

    //  >>>>> Update Windows Group Topics >>>>>>>>>
    for (uint8_t i = 0; i < controller.get_ent_counter(WIN_ENT); i++)
    {
      Win_props win_props;
      controller.get_entity_prop(WIN_ENT, i, win_props);
      if (!win_props.virtCMD)
      {
        _updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);
        accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
        break;
      }
    }

    //  >>>>> Update Switch Group Topics >>>>>>>>>
    for (uint8_t i = 0; i < controller.get_ent_counter(SW_ENT); i++)
    {
      SW_props sw_props;
      controller.get_entity_prop(SW_ENT, i, sw_props);
      if (!sw_props.virtCMD)
      {
        _updateTopics_flash(DOC, SwGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);
        accum_shift += sizeof(SwGroupTopics) / (sizeof(SwGroupTopics[0]));
        break;
      }
    }

    //  >>>>> Update Windows Topics >>>>>>>>>
    for (uint8_t i = 0; i < controller.get_ent_counter(WIN_ENT); i++)
    {
      Win_props win_prop;
      controller.get_entity_prop(WIN_ENT, i, win_prop);
      if (!win_prop.virtCMD)
      {
        controller.set_ent_name(i, WIN_ENT, DOC["sub_topics_win"][i].as<const char *>());
        iot.topics_sub[accum_shift + i] = controller.get_ent_name(i, WIN_ENT);
      }
    }
    accum_shift += controller.get_ent_counter(WIN_ENT);

    //  >>>>> Update Switch Topics >>>>>>>>>
    for (uint8_t i = 0; i < controller.get_ent_counter(SW_ENT); i++)
    {
      SW_props sw_prop;
      controller.get_entity_prop(SW_ENT, i, sw_prop);
      if (!sw_prop.virtCMD)
      {
        controller.set_ent_name(i, SW_ENT, DOC["sub_topics_SW"][i].as<const char *>());
        iot.topics_sub[accum_shift + i] = controller.get_ent_name(i, SW_ENT);
      }
    }
    Serial.println(F("~ Topics build - OK "));
    return 1;
  }
  else
  {
    Serial.println(F("~ Topics build - Failed "));
    return 0;
  }
}
bool get_pins_parameters(JsonDocument &DOC, uint8_t _inpins[], uint8_t _outpins[], long _RFcodes[], uint8_t &in, uint8_t &out, uint8_t &RF, uint8_t &RFp)
{
#if MAN_MODE == true
  if (getPins_manual(DOC))
  {
    Serial.println(F("~ Read pins hardware - local"));
#else
  if (iot.readJson_inFlash(DOC, iot.parameter_filenames[1]))
  {
    Serial.println(F("~ Read pins hardware - flash"));
#endif
    RFp = DOC["RFpin"];
    in = DOC["inputPins"].size();
    out = DOC["relayPins"].size();
    RF = DOC["RF_keyboardCode"].size();

    for (uint8_t i = 0; i < RF; i++)
    {
      _RFcodes[i] = DOC["RF_keyboardCode"][i] | 255;
    }
    for (uint8_t i = 0; i < in; i++)
    {
      _inpins[i] = DOC["inputPins"][i] | 255;
    }
    for (uint8_t i = 0; i < out; i++)
    {
      _outpins[i] = DOC["relayPins"][i] | 255;
    }

    Serial.println(F("~ Hardware pins read - OK "));
    return 1;
  }
  else
  {
    Serial.println(F("~ Hardware pins read - Failed "));
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
  Serial.println(F("~ IOT2 parameters    - local"));
  return 1;
#else
  StaticJsonDocument<1400> DOC;
  if (iot.readFlashParameters(DOC, iot.parameter_filenames[0]))
  {
    Serial.println("~ IOT2 parameters read - OK");
    return 1;
  }
  else
  {
    Serial.println("~ IOT2 parameters read - Failed");
    return 0;
  }
#endif
}
bool get_entities_parameters()
{
  StaticJsonDocument<1500> DOC;

  long _RFcodes[10]{};
  uint8_t _inpins[12]{};
  uint8_t _outpins[8]{};
  uint8_t RF_p = 0;
  uint8_t actual_inpins_saved = 0;
  uint8_t actual_outpins_saved = 0;
  uint8_t actual_RFcodes_saved = 0;

  if (get_pins_parameters(DOC, _inpins, _outpins, _RFcodes, actual_inpins_saved, actual_outpins_saved, actual_RFcodes_saved, RF_p))
  {

#if MAN_MODE == true
    Serial.println(F("~ entities data-base - local"));
    DeserializationError error0 = deserializeJson(DOC, cont_params);
    if (!error0)
    {
#else
    Serial.println(F("~ entities data-base - flash"));
    if (iot.readJson_inFlash(DOC, iot.parameter_filenames[3]))
    {
#endif
      JsonArray entTypes = DOC["entityType"];
      uint8_t win_ents = 0;
      uint8_t sw_ents = 0;

      controller.set_RF(RF_p);                             /* Set RF HW Pin */
      controller.set_RFch(_RFcodes, actual_RFcodes_saved); /* Set RF frequencies */

      for (uint8_t x = 0; x < entTypes.size(); x++) /* Creating Entities */
      {
        if (entTypes[x].as<uint8_t>() == WIN_ENT) /* win Entity */
        {
          controller.create_Win(_inpins, _outpins, DOC["Winname"][win_ents].as<const char *>(),
                                DOC["WinvirtCMD"][win_ents].as<bool>(), DOC["WextInputs"][win_ents].as<bool>());
          win_ents++;
        }
        else if (entTypes[x].as<uint8_t>() == SW_ENT) /* SW entity */
        {
          controller.create_SW(_inpins, _outpins, DOC["SWname"][sw_ents].as<const char *>(),
                               DOC["SW_buttonTypes"][sw_ents].as<uint8_t>(), DOC["SWvirtCMD"][sw_ents].as<bool>(),
                               DOC["SW_timeout"][sw_ents].as<int>(), DOC["RF_2entity"][sw_ents].as<uint8_t>());
          sw_ents++;
        }
      }
      Serial.println(F("~ Entities build - OK "));
      return 1;
    }
    else
    {
      Serial.println(F("~ Entities read file - Failed "));
      return 0;
    }
  }
  else
  {
    Serial.println(F("~ Entities build - Failed "));
    return 0;
  }
}

void read_all_parameters()
{
  Serial.println(F("\n±±±±±±±±±±±± Start Reading Parameters n±±±±±±±±±±±±"));

#if MAN_MODE == false
  build_path_directory(PARAM_PRESET); /* Needed for Flash only */
#endif

  if (get_IOT2_parameters() && get_entities_parameters() && readTopics())
  {
    bootProcess_OK = true;
    Serial.println(F("\n±±±±±±±±±±±± Reading Parameters -OK  n±±±±±±±±±±±±"));
  }
  else
  {
    Serial.println(F("\n±±±±±±±±±±±± Reading Parameters -Failed  n±±±±±±±±±±±±"));

    bootProcess_OK = false;
    uint8_t fail_directory = 0;
    build_path_directory(fail_directory);
    get_IOT2_parameters();
    readTopics();
  }
}