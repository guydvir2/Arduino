myIOT2 iot;
#define LOCAL_PARAM true

extern void create_SW_instance(JsonDocument &_DOC, uint8_t i);
extern void create_WinSW_instance(JsonDocument &_DOC, uint8_t i);

bool paramLoadedOK = false;
char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[2][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

/* ±±±±±±±±± Filenames and directories for each controller ±±±±±±±±±±± */
char parameterFiles[4][30];
const char *dirs[] = {"Fail", "Cont_A", "Cont_B", "Cont_C", "Cont_D", "Cont_test"};
const char *FileNames_common[2] = {"myIOT_param.json", "Hardware.json"};
const char *FileNames_dedicated[2] = {"myIOT2_topics.json", "sketch_param.json"};

void construct_filenames(uint8_t i = 0)
{
  uint8_t tot_num_files = 4;
  uint8_t num_common_files = 2;

  for (uint8_t x = 0; x < tot_num_files; x++)
  {
    if (x < num_common_files)
    {
      sprintf(parameterFiles[x], "/%s", FileNames_common[x]);
    }
    else
    {
      sprintf(parameterFiles[x], "/%s/%s", dirs[i], FileNames_dedicated[x - num_common_files]);
    }
    iot.parameter_filenames[x] = parameterFiles[x];
  }
}
/* ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±± */

/* ±±±±±±±±±±±±±± Read & Update Paramters stored on flash memory ±±±±±±±±±±±±±±±± */
void _updateTopics_flash(JsonDocument &DOC, char ch_array[], const char *dest_array[], const char *topic, u_int8_t i, uint8_t shift = 0)
{
  strlcpy(ch_array, DOC[topic][i], MAX_TOPIC_SIZE);
  dest_array[i + shift] = ch_array;
}
void _updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0)
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

void _update_HWpins_flash(JsonDocument &DOC)
{
  const uint8_t num_RF_codes = 4;
  RFpin = DOC["RFpin"];

  for (uint8_t i = 0; i < num_RF_codes; i++)
  {
    RF_keyboardCode[i] = DOC["RF_keyboardCode"][i].as<int>() | 0;
  }
  for (uint8_t i = 0; i < TOT_Inputs; i++)
  {
    inPinsArray[i] = DOC["inputPins"][i] | 255;
  }
  for (uint8_t i = 0; i < TOT_Relays; i++)
  {
    outPinsArray[i] = DOC["relayPins"][i] | 255;
  }
}
void _update_sketch_parameters_flash(JsonDocument &DOC)
{
  JsonArray entTypes = DOC["entityType"];

  for (uint8_t x = 0; x < entTypes.size(); x++)
  {
    if (entTypes[x] == 0)
    {
      /* create Window instance */
      create_WinSW_instance(DOC, x);
    }
    else if (entTypes[x] == 1)
    {
      /* create Window instance */
      create_SW_instance(DOC, x);
    }
  }
}
void update_Parameters_flash()
{
  StaticJsonDocument<800> DOC;

  Serial.begin(115200);
  delay(100);

  bool ok0 = false;
  bool ok1 = false;
  bool ok2 = false;
  bool ok3 = false;

  /* ±±±±±±±± Part A_0: update filenames of parameter files ±±±±±±±±±±±*/
  construct_filenames(ParameterFile_preset);

  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  /* ±±±±±±±± Part A_1: Update HW pins ±±±±±±±±±±±*/
#if LOCAL_PARAM
  char fakePin[] = "{\"inputPins\": [19,17,16,5,4,2,23,18,15,14,13,12], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
  DeserializationError error0 = deserializeJson(DOC, fakePin);
  if (!error0)
  {
    _update_HWpins_flash(DOC);
    DOC.clear();
    ok0 = true;
  }
#else
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC))
  {
    _update_HWpins_flash(DOC);
    DOC.clear();
    ok0 = true;
  }
  else
  {
    Serial.println("Fail read HW pins");
  }
#endif
  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  /* ±±±±±±±± Part B: update myIOT parameters ±±±±±±±±±±±*/
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC))
  {
    iot.update_vars_flash_parameters(DOC);
    DOC.clear();
    ok1 = true;
  }
  else
  {
    Serial.println("Fail read myIOT2 parameters");
  }

  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  // /* ±±±±±±±±±± Part C: update Sketch Parameters ±±±±±±±±±±±±±± */
#if LOCAL_PARAM
  char fakeP[] = "{\"entityType\": [0, 0],\
                    \"virtCMD\": [\"\",\"myHome\/Windows\/gFloor\/W0\",\"myHome\/Lights\/int\/gFloor\/SW0\",\"\"],\
                    \"SW_buttonTypes\": [1,2,1],\
                    \"WextInputs\": [1,0],\
                    \"SW_RF\": [0,1,2,3],\
                    \"SW_timeout\": [10,15,0,0],\
                    \"v_file\": 0.4}";
  DeserializationError error2 = deserializeJson(DOC, fakeP);
  if (!error2)
  {
    _update_sketch_parameters_flash(DOC);
    DOC.clear();
    ok2 = true;
  }
#else
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[3], DOC))
  {
    _update_sketch_parameters_flash(DOC);
    DOC.clear();
    ok2 = true;
  }
  else
  {
    Serial.println("Fail read Sketch parameters");
  }
#endif
  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  // /* ±±±±±±±±±± Part D: update myIOT Topics ±±±±±±±±±± */
#if LOCAL_PARAM
  char fakeTopics[] = "{\
                        \"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\
                        \"pub_topics\" : [\"myHome\/Cont_Man\/Avail\", \"myHome\/Cont_Man\/State\"],\
                        \"sub_topics\" : [\"myHome\/Cont_Man\",\"myHome\/All\",\"myHome\/lockdown\"],\
                        \"sub_topics_win\" : [\"myHome\/Windows\/gFloor\/Win0\",\"myHome\/Windows\/gFloor\/Win1\",\"myHome\/Windows\/gFloor\/Win2\"],\
                        \"sub_topics_SW\" : [\"myHome\/Light\/int\/gFloor\/Light0\",\"myHome\/Light\/int\/gFloor\/Light1\",\"myHome\/Light\/int\/gFloor\/Light2\",\"myHome\/Light\/int\/gFloor\/Light3\"],\
                        \"sub_topics_win_g\" : [\"myHome\/Windows\",\"myHome\/Windows\/gFloor\"],\
                        \"sub_topics_SW_g\" : [\"myHome\/Light\",\"myHome\/Light\/int\",\"myHome\/Light\/int\/gFloor\"],\
                        \"ver\" : 0.1}";

  DeserializationError error3 = deserializeJson(DOC, fakeTopics);
  if (!error3)
  {
    _updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    _updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
    _updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

    //  >>>>> Update Windows Group Topics >>>>>>>>>
    uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
    _updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);

    //  >>>>> Update Switch Group Topics >>>>>>>>>
    accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
    _updateTopics_flash(DOC, buttGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);

    //  >>>>> Update Windows Topics >>>>>>>>>
    accum_shift += sizeof(buttGroupTopics) / (sizeof(buttGroupTopics[0]));
    for (uint8_t i = 0; i < winEntityCounter; i++)
    {
      if (winSW_V[i]->virtCMD == false)
      {
        _updateTopics_flash(DOC, winSW_V[i]->name, iot.topics_sub, "sub_topics_win", i, accum_shift + i);
      }
    }

    //  >>>>> Update Switch Topics >>>>>>>>>
    accum_shift += winEntityCounter + 1;
    for (uint8_t i = 0; i < swEntityCounter; i++)
    {
      if (SW_v[i]->is_virtCMD() == false)
      {
        _updateTopics_flash(DOC, SW_v[i]->name, iot.topics_sub, "sub_topics_SW", i, accum_shift + i);
      }
    }
    DOC.clear();
    ok3 = true;
  }
#else
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC)) /* extract topics from flash */
  {
    _updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    _updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
    _updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

    //  >>>>> Update Windows Group Topics >>>>>>>>>
    uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
    _updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);

    //  >>>>> Update Switch Group Topics >>>>>>>>>
    accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
    _updateTopics_flash(DOC, buttGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);

    //  >>>>> Update Windows Topics >>>>>>>>>
    accum_shift += sizeof(buttGroupTopics) / (sizeof(buttGroupTopics[0]));
    for (uint8_t i = 0; i < winEntityCounter; i++)
    {
      _updateTopics_flash(DOC, winSW_V[i]->name, iot.topics_sub, "sub_topics_win", i, accum_shift + i);
    }

    //  >>>>> Update Switch Topics >>>>>>>>>
    accum_shift += winEntityCounter;
    for (uint8_t i = 0; i < swEntityCounter; i++)
    {
      _updateTopics_flash(DOC, SW_v[i]->name, iot.topics_sub, "sub_topics_SW", i, accum_shift + i);
    }
    DOC.clear();
    ok3 = true;
  }
  else
  {
    /* Tpoics in case of failure getting flash parameters */
    sprintf(topics_gen_pub[0], "myHome/Messages");
    sprintf(topics_gen_pub[1], "myHome/log");
    sprintf(topics_sub[0], "myHome/Cont_Err");
    sprintf(topics_pub[0], "myHome/Cont_Err/State");

    iot.topics_gen_pub[0] = topics_gen_pub[0];
    iot.topics_gen_pub[1] = topics_gen_pub[1];
    iot.topics_pub[0] = topics_pub[0];
    iot.topics_sub[0] = topics_sub[0];

    Serial.println("Fail read Topics parameter");
  }
#endif
  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  paramLoadedOK = ok0 && ok1 && ok2 && ok3;
  if (paramLoadedOK)
  {
    Serial.println("~ Parameter files - read OK");
  }
  else
  {
    Serial.println("~ Parameter files - read failed");
  }
}
/* ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±± */

void win_updateState(uint8_t i, uint8_t state) /* Windows State MQTT update */
{
  char t[60];
  char r[5];
  sprintf(t, "%s/State", winSW_V[i]->name);
  sprintf(r, "%d", state);
  iot.pub_noTopic(r, t, true);
}
void SWupdateState(uint8_t i, bool state) /* Button State MQTT update */
{
  char t[60];
  char r[5];
  sprintf(t, "%s/State", SW_v[i]->name);
  sprintf(r, "%d", state);
  iot.pub_noTopic(r, t, true);
}
void _gen_WinMSG(uint8_t state, uint8_t reason, uint8_t i)
{
  char msg[100];
  sprintf(msg, "[%s]: [WIN#%d] [%s] turned [%s]", REASONS_TXT[reason], i, winSW_V[i]->name, STATES_TXT[state]);
  iot.pub_msg(msg);
  win_updateState(i, state);
}
void _gen_SW_MSG(uint8_t i, uint8_t type, bool request)
{
  char msg[100];
  sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s]", SW_Types[type], i, SW_v[i]->name, request == HIGH ? "ON" : "OFF");
  iot.pub_msg(msg);
  SWupdateState(i, (int)request);
}
void _post_Win_virtCMD(uint8_t state, uint8_t reason, uint8_t x)
{
  if (winSW_V[x]->virtCMD == true)
  {
    iot.pub_noTopic((char *)winMQTTcmds[state], winSW_V[x]->name); // <---- Fix this : off cmd doesnot appear
  }
}

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
  char msg[150];
  if (strcmp(incoming_msg, "status") == 0)
  {
    if (winEntityCounter > 0)
    {
      for (uint8_t i = 0; i < winEntityCounter; i++)
      {
        sprintf(msg, "Status: Win[#%d][%s] [%s]%s", i, winSW_V[i]->name, winMQTTcmds[winSW_V[i]->get_winState()], i == (swEntityCounter - 1) ? "" : "; ");
        iot.pub_msg(msg);
      }
    }
    if (swEntityCounter > 0)
    {
      for (uint8_t i = 0; i < swEntityCounter; i++)
      {
        sprintf(msg, "Status: SW[#%d][%s] [%s]%s", i, SW_v[i]->name, i, SW_v[i]->get_SWstate() ? "On" : "Off", i == swEntityCounter - 1 ? "" : "; ");
        iot.pub_msg(msg);
      }
    }
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "help #2:No other functions");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "ver #2:");
    iot.pub_msg(msg);
  }
  else if (strcmp(winGroupTopics[1], _topic) == 0) /* lockdown */
  {
    if (strcmp(incoming_msg, "true") == 0)
    {
      for (uint8_t i = 0; i < winEntityCounter; i++)
      {
        iot.pub_msg("Lockdown: Start");
        winSW_V[i]->init_lockdown();
      }
    }
    if (strcmp(incoming_msg, "false") == 0)
    {
      for (uint8_t i = 0; i < winEntityCounter; i++)
      {
        iot.pub_msg("Lockdown: Released");
        winSW_V[i]->release_lockdown();
      }
    }
  }
  else
  {
    if (strcmp(iot.inline_param[0], EntTypes[0]) == 0 && /* Windows */
        (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0 || strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0 || strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0))
    {
      uint8_t _word = 0;
      if (strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0) /* UP */
      {
        _word = UP;
      }
      else if (strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0) /* DOwN */
      {
        _word = DOWN;
      }
      else if (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0) /* STOP */
      {
        _word = STOP;
      }

      for (uint8_t i = 0; i < winEntityCounter; i++)
      {
        if (strcmp(_topic, winSW_V[i]->name) == 0) /* SENT FOR A SPECIFIC TOPIC */
        {
          Serial.println("gotit");
          winSW_V[i]->set_WINstate(_word, MQTT);
          return;
        }
      }
      for (uint8_t i = 0; i < sizeof(winGroupTopics) / sizeof(winGroupTopics[0]); i++) /* OR SENT FOR A GROUP */
      {
        if (strcmp(_topic, winGroupTopics[i]) == 0)
        {
          for (uint8_t i = 0; i < winEntityCounter; i++)
          {
            winSW_V[i]->set_WINstate(_word, MQTT);
          }
        }
      }
    }
    else if (strcmp(iot.inline_param[0], EntTypes[1]) == 0 && (strcmp(iot.inline_param[1], SW_MQTT_cmds[0]) == 0 || strcmp(iot.inline_param[1], SW_MQTT_cmds[1]) == 0)) /* MQTT cmd for SW */
    {
      for (uint8_t i = 0; i < swEntityCounter; i++)
      {
        if (strcmp(_topic, SW_v[i]->name) == 0) /* SENT FOR A SPECIFIC TOPIC */
        {
          if (strcmp(iot.inline_param[1], SW_MQTT_cmds[0]) == 0) /* ON */
          {
            SW_v[i]->turnON_cb(MQTT);
            return;
          }
          else if (strcmp(iot.inline_param[1], SW_MQTT_cmds[1]) == 0) /* OFF */
          {
            SW_v[i]->turnOFF_cb(MQTT);
            return;
          }
        }
      }
      for (uint8_t i = 0; i < sizeof(buttGroupTopics) / sizeof(buttGroupTopics[0]); i++) /* SENT FOR A GROUP TOPIC */
      {
        if (strcmp(_topic, buttGroupTopics[i]) == 0)
        {
          for (uint8_t i = 0; i < swEntityCounter; i++)
          {
            if (strcmp(iot.inline_param[1], SW_MQTT_cmds[0]) == 0) /* ON */
            {
              SW_v[i]->turnON_cb(MQTT);
            }
            else if (strcmp(iot.inline_param[1], SW_MQTT_cmds[1]) == 0) /* OFF */
            {
              SW_v[i]->turnOFF_cb(MQTT);
            }
          }
          return;
        }
      }
    }
  }
}
void startIOTservices()
{
  update_Parameters_flash();
  iot.start_services(addiotnalMQTT);
}