myIOT2 iot;

extern bool _isON(uint8_t i);
extern void SW_turnON_cb(uint8_t i, uint8_t type);
extern void SW_turnOFF_cb(uint8_t i, uint8_t type);
extern void create_SW_instance(JsonDocument &_DOC, uint8_t i);
extern void create_WinSW_instance(JsonDocument &_DOC, uint8_t i);

bool paramLoadedOK = false;
char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[2][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

/* ±±±±±±±±± Filenames and directories for each controller ±±±±±±±±±±± */
char parameterFiles[3][30];
const char *dirs[] = {"Fail", "Cont_A", "Cont_B", "Cont_C", "Cont_D"};
const char *FileNames[3] = {"myIOT_param.json", "myIOT2_topics.json", "sketch_param.json"};

void construct_filenames(uint8_t i = 0)
{
  for (uint8_t x = 0; x < 3; x++)
  {
    sprintf(parameterFiles[x], "/%s/%s", dirs[i], FileNames[x]);
    iot.parameter_filenames[x] = parameterFiles[x];
  }
}
/* ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±± */

/* ±±±±±±±±±±±±±± Read & Update Paramters stored on flash memory ±±±±±±±±±±±±±±±± */
void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, uint8_t shift = 0)
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
void updateTopics_flash(JsonDocument &DOC, char ch_array[], const char *dest_array[], const char *topic, u_int8_t i, uint8_t shift = 0)
{
  strlcpy(ch_array, DOC[topic][i], MAX_TOPIC_SIZE);
  dest_array[i + shift] = ch_array;
}
void update_sketch_parameters_flash(JsonDocument &DOC)
{
  RFpin = DOC["RFpin"].as<uint8_t>();

  for (uint8_t i = 0; i < 4; i++)
  {
    RF_keyboardCode[i] = DOC["RF_keyboardCode"][i].as<int>();
  }

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
  StaticJsonDocument<1200> DOC;

  Serial.begin(115200);
  bool ok1 = false;
  bool ok2 = false;
  bool ok3 = false;

  /* ±±±±±±±± Part A: update filenames of paramter files ±±±±±±±±±±±*/
  construct_filenames(CONTROLLER_PRESET);

  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  /* ±±±±±±±± Part B: Read from flash, and update myIOT parameters ±±±±±±±±±±±*/
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC))
  {

    // char fakeIOT[] = "{\"useSerial\": true,\"useWDT\" : true,\"useOTA\" : true,\"useFlashP\" : true,\"useResetKeeper\" : true,\"useDebugLog\" : true,\"useNetworkReset\" : true,\"useBootClockLog\" : true,\"ignore_boot_msg\" : false,\"debug_level\" : 0,\"noNetwork_reset\" : 30,\"ver\" : 0.6}";
    // DeserializationError error = deserializeJson(DOC, fakeIOT);
    // if (!error)
    // {
    iot.update_vars_flash_parameters(DOC);
    DOC.clear();
    ok1 = true;
  }
  
  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  // /* ±±±±±±±±±± Part C: Read Sketch paramters from flash, and update Sketch ±±±±±±±±±±±±±± */
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC))
  {
    // char fakeP[] = "{\"entityType\": [0,1,1],\"virtCMD\": [\"\",\"\",\"\",\"\"],\"inputPins\": [19,17,16,5,4,2,23,18,15,14,13,12],\"relayPins\": [25,26,33,32],\"SW_buttonTypes\": [1,1,2,1],\"WextInputs\": [1,0,0],\"RF_keyboardCode\":[3135496,3135492,3135490,3135489],\"RFpin\": 27,\"v_file\": 0.2}";
    // DeserializationError error2 = deserializeJson(DOC, fakeP);
    // if (!error2)
    // {
    update_sketch_parameters_flash(DOC);
    DOC.clear();
    ok2 = true;
  }
  
  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  // /* ±±±±±±±±±± Part D: Read Topics from flash, and update myIOT Topics ±±±±±±±±±± */
  // char fakeTopics[] = "{\"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\"pub_topics\" : [\"myHome\/Cont_A\/Avail\",\"myHome\/Cont_A\/State\"],\"sub_topics\" : [\"myHome\/Cont_A\",\"myHome\/All\",\"myHome\/lockdown\"],\"sub_topics_win\" : [\"myHome\/Windows\/gFloor\/TwinWindow\",\"myHome\/Windows\/gFloor\/Pergola\"],\"sub_topics_SW\" : [\"myHome\/Light\/int\/gFloor\/SalAmbient1\",\"myHome\/Light\/int\/gFloor\/SalAmbient2\",\"myHome\/Light\/int\/gFloor\/Lobby\",\"myHome\/Light\/int\/gFloor\/Corridor\"],\"sub_topics_win_g\" : [\"myHome\/Windows\",\"myHome\/Windows\/gFloor\"],\"sub_topics_SW_g\" : [\"myHome\/Light\",\"myHome\/Light\/int\",\"myHome\/Light\/int\/gFloor\"],\"ver\" : 0.1}";
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC)) /* extract topics from flash */
  {
    // DeserializationError error3 = deserializeJson(DOC, fakeTopics);
    // if (!error3)
    // {
    updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
    updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");
    
    //  >>>>> Update Windows Group Topics >>>>>>>>>
    uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
    updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);

    //  >>>>> Update Switch Group Topics >>>>>>>>>
    accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
    updateTopics_flash(DOC, buttGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);

    //  >>>>> Update Windows Topics >>>>>>>>>
    accum_shift += sizeof(buttGroupTopics) / (sizeof(buttGroupTopics[0]));
    for (uint8_t i = 0; i < winEntityCounter; i++)
    {
      updateTopics_flash(DOC, winSW_V[i]->name, iot.topics_sub, "sub_topics_win", i, accum_shift + i);
    }

    //  >>>>> Update Switch Topics >>>>>>>>>
    accum_shift += winEntityCounter;
    for (uint8_t i = 0; i < swEntityCounter; i++)
    {
      updateTopics_flash(DOC, SW_v[i]->Topic, iot.topics_sub, "sub_topics_SW", i, accum_shift + i);
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
  }
  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  paramLoadedOK = ok1 && ok2 && ok3;
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
void butt_updateState(uint8_t i, bool state) /* Button State MQTT update */
{
  char t[60];
  char r[5];
  sprintf(t, "%s/State", SW_v[i]->Topic);
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
void _gen_ButtMSG(uint8_t i, uint8_t type, bool request)
{
  char msg[100];
  sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s]", turnTypes[type], i, SW_v[i]->Topic, request == HIGH ? "ON" : "OFF");
  iot.pub_msg(msg);
  butt_updateState(i, (int)request);
}

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
  char msg[150];
  if (strcmp(incoming_msg, "status") == 0)
  {
    char msg2[100];
    strcpy(msg, "Status: ");
    if (winEntityCounter > 0)
    {
      strcat(msg, "[Windows]- ");
      for (uint8_t i = 0; i < winEntityCounter; i++)
      {
        sprintf(msg2, " Win[#%d][%s] [%d]%s", i, winSW_V[i]->name, winSW_V[i]->get_winState(), i == swEntityCounter - 1 ? "" : "; ");
        strcat(msg, msg2);
      }
      iot.pub_msg(msg);
    }
    strcpy(msg, "Status: ");
    if (swEntityCounter > 0)
    {
      strcat(msg, "[Switches]- ");
      for (uint8_t i = 0; i < swEntityCounter; i++)
      {
        sprintf(msg2, " SW[#%d][%s] [%d]%s", i, SW_v[i]->Topic, _isON(i), i == swEntityCounter - 1 ? "" : "; ");
        strcat(msg, msg2);
      }
      iot.pub_msg(msg);
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
      if (strcmp(iot.inline_param[1], winMQTTcmds[0]) == 0) /* UP */
      {
        _word = UP;
      }
      else if (strcmp(iot.inline_param[1], winMQTTcmds[1]) == 0) /* DOwN */
      {
        _word = DOWN;
      }
      else if (strcmp(iot.inline_param[1], winMQTTcmds[2]) == 0) /* STOP */
      {
        _word = STOP;
      }

      for (uint8_t i = 0; i < winEntityCounter; i++)
      {
        if (strcmp(_topic, winSW_V[i]->name) == 0) /* SENT FOR A SPECIFIC TOPIC */
        {
          Serial.println("gotit");
          winSW_V[i]->ext_SW(_word, MQTT);
          return;
        }
      }
      for (uint8_t i = 0; i < sizeof(winGroupTopics) / sizeof(winGroupTopics[0]); i++) /* OR SENT FOR A GROUP */
      {
        if (strcmp(_topic, winGroupTopics[i]) == 0)
        {
          for (uint8_t i = 0; i < winEntityCounter; i++)
          {
            winSW_V[i]->ext_SW(_word, MQTT);
          }
        }
      }
    }
    else if (strcmp(iot.inline_param[0], EntTypes[1]) == 0 && (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0 || strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0)) /* MQTT cmd for SW */
    {
      for (uint8_t i = 0; i < swEntityCounter; i++)
      {
        if (strcmp(_topic, SW_v[i]->Topic) == 0) /* SENT FOR A SPECIFIC TOPIC */
        {
          if (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0) /* ON */
          {
            SW_turnON_cb(i, _MQTT);
            return;
          }
          else if (strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0) /* OFF */
          {
            SW_turnOFF_cb(i, _MQTT);
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
            if (strcmp(iot.inline_param[1], buttMQTTcmds[0]) == 0) /* ON */
            {
              SW_turnON_cb(i, _MQTT);
            }
            else if (strcmp(iot.inline_param[1], buttMQTTcmds[1]) == 0) /* OFF */
            {
              SW_turnOFF_cb(i, _MQTT);
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