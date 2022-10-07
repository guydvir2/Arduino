myIOT2 iot;
#define RETAINED_MSG false

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

  bool ok0 = false;
  bool ok1 = false;
  bool ok2 = false;
  bool ok3 = false;

  /* ±±±±±±±± Part A_0: filenames of parameter files ±±±±±±±±±±±*/
  construct_filenames(ParameterFile_preset);

  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  /* ±±±±±±±± Part A_1: HW pins ±±±±±±±±±±±*/
#if LOCAL_PARAM
  DeserializationError error0 = deserializeJson(DOC, fakePin);
  if (!error0)
  {
#else
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC))
  {
#endif
    _update_HWpins_flash(DOC);
    DOC.clear();
    ok0 = true;
  }
  else
  {
    Serial.println("Fail read HW pins");
  }

  // ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

  /* ±±±±±±±± Part B: myIOT parameters ±±±±±±±±±±±*/
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

// /* ±±±±±±±±±± Part C: Sketch Parameters ±±±±±±±±±±±±±± */
#if LOCAL_PARAM
  DeserializationError error2 = deserializeJson(DOC, fakeP);
  if (!error2)
  {
#else
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[3], DOC))
  {
#endif
    _update_sketch_parameters_flash(DOC);
    DOC.clear();
    ok2 = true;
  }
  else
  {
    Serial.println("Fail read Sketch parameters");
  }

// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

// /* ±±±±±±±±±± Part D: myIOT Topics ±±±±±±±±±± */
#if LOCAL_PARAM
  DeserializationError error3 = deserializeJson(DOC, fakeTopics);
  if (!error3)
  {
#else
  if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC)) /* extract topics from flash */
  {
#endif
    //     _updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    //     _updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
    //     _updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

    //     //  >>>>> Update Windows Group Topics >>>>>>>>>
    //     uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
    //     _updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);

    //     //  >>>>> Update Switch Group Topics >>>>>>>>>
    //     accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
    //     _updateTopics_flash(DOC, SwGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);

    //     //  >>>>> Update Windows Topics >>>>>>>>>
    //     accum_shift += sizeof(SwGroupTopics) / (sizeof(SwGroupTopics[0]));
    //     for (uint8_t i = 0; i < winEntityCounter; i++)
    //     {
    //       if (winSW_V[i]->virtCMD == false)
    //       {
    //         _updateTopics_flash(DOC, winSW_V[i]->name, iot.topics_sub, "sub_topics_win", i, accum_shift + i);
    //       }
    //     }

    //     //  >>>>> Update Switch Topics >>>>>>>>>
    //     accum_shift += winEntityCounter + 1;
    //     for (uint8_t i = 0; i < swEntityCounter; i++)
    //     {
    //       if (SW_v[i]->is_virtCMD() == false)
    //       {
    //         _updateTopics_flash(DOC, SW_v[i]->name, iot.topics_sub, "sub_topics_SW", i, accum_shift + i);
    //       }
    //     }
    //     DOC.clear();
    //     ok3 = true;
    //   }
    // #else
    // if (iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC)) /* extract topics from flash */
    // {
    _updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics");
    _updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics");
    _updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics");

    //  >>>>> Update Windows Group Topics >>>>>>>>>
    uint8_t accum_shift = sizeof(topics_sub) / (sizeof(topics_sub[0]));
    _updateTopics_flash(DOC, winGroupTopics, iot.topics_sub, "sub_topics_win_g", accum_shift);

    //  >>>>> Update Switch Group Topics >>>>>>>>>
    accum_shift += sizeof(winGroupTopics) / (sizeof(winGroupTopics[0]));
    _updateTopics_flash(DOC, SwGroupTopics, iot.topics_sub, "sub_topics_SW_g", accum_shift);

    //  >>>>> Update Windows Topics >>>>>>>>>
    accum_shift += sizeof(SwGroupTopics) / (sizeof(SwGroupTopics[0]));
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
  // #endif
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

void Winupdate_MQTT_state(uint8_t i, uint8_t state) /* Windows State MQTT update */
{
  char t[60];
  char r[5];
  sprintf(t, "%s/State", winSW_V[i]->name);
  sprintf(r, "%d", state);
  iot.pub_noTopic(r, t, true);
}
void SWupdate_MQTT_state(uint8_t i, bool state) /* Button State MQTT update */
{
  char t[60];
  char r[5];
  sprintf(t, "%s/State", SW_v[i]->name);
  sprintf(r, "%d", state);
  iot.pub_noTopic(r, t, true);
}
void Win_send_MQTT_switch(uint8_t state, uint8_t reason, uint8_t i)
{
  char msg[100];
  sprintf(msg, "[%s]: [WIN#%d] [%s] turned [%s]", REASONS_TXT[reason], i, winSW_V[i]->name, STATES_TXT[state]);
  iot.pub_msg(msg);
#if RETAINED_MSG
  iot.pub_noTopic((char *)winMQTTcmds[state], winSW_V[i]->name, true); /* Retain Message */
#endif
  Winupdate_MQTT_state(i, state); /* Retain State */
}
void SW_send_MQTT_switch(uint8_t i, uint8_t type, bool request)
{
  char msg[100];
  char msg2[30];

  if (request == HIGH && SW_v[i]->useTimeout())
  {
    char t[20];
    iot.convert_epoch2clock((int)((SW_v[i]->get_remain_time() + 500) / 1000), 0, t);
    sprintf(msg2, "timeout [%s]", t);
  }
  else if (!SW_v[i]->useTimeout())
  {
    strcpy(msg2, "timeout [No]");
  }
  else if (request == LOW)
  {
    strcpy(msg2, "");
  }

  sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s] %s", SW_Types[type], i, SW_v[i]->name, request == HIGH ? "ON" : "OFF", msg2);
  iot.pub_msg(msg);

  SWupdate_MQTT_state(i, (int)request);
}
void Win_send_virtCMD(uint8_t state, uint8_t reason, uint8_t x)
{
  if (winSW_V[x]->virtCMD == true)
  {
    iot.pub_noTopic((char *)winMQTTcmds[state], winSW_V[x]->name, true); // <---- Fix this : off cmd doesnot appear
  }
}
void SW_send_virtCMD(smartSwitch &sw)
{
  char msg[100];
  iot.pub_noTopic((char *)SW_MQTT_cmds[sw.telemtryMSG.state], sw.name, true); /* Retained */
  sprintf(msg, "[%s]: Switched [%s] Virtual [%s]", SW_Types[sw.telemtryMSG.reason], SW_MQTT_cmds[sw.telemtryMSG.state], sw.name);
  iot.pub_msg(msg);
}
/* ±±±±±±±±±±±±±± Receiving and handling MQTT CMDs ±±±±±±±±±±±±±±±± */

/* CHECK TOPICS - for what entity it belongs */
bool MQTT_is_Contl_topic(char *_topic)
{
  if (strcmp(_topic, topics_sub[0]) == 0 /* Cont Topic */ || strcmp(_topic, topics_sub[1]) == 0 /* All Topic */)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
bool MQTT_is_lockdown_topic(char *_topic)
{
  if (strcmp(_topic, topics_sub[2]) == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
bool MQTT_is_Win_topic(char *_topic, uint8_t &n)
{
  for (uint8_t i = 0; i < winEntityCounter; i++)
  {
    if (strcmp(_topic, winSW_V[i]->name) == 0) /* SENT FOR A SPECIFIC TOPIC */
    {
      n = i;
      return 1;
    }
  }
  return 0;
}
bool MQTT_is_SW_topic(char *_topic, uint8_t &n)
{
  for (uint8_t i = 0; i < swEntityCounter; i++)
  {
    if (strcmp(_topic, SW_v[i]->name) == 0) /* SENT FOR A SPECIFIC TOPIC */
    {
      n = i;
      return 1;
    }
  }
  return 0;
}
bool MQTT_is_Wingroup_topic(char *_topic)
{
  for (uint8_t i = 0; i < sizeof(winGroupTopics) / sizeof(winGroupTopics[0]); i++) /* OR SENT FOR A GROUP */
  {
    if (strcmp(_topic, winGroupTopics[i]) == 0)
    {
      return 1;
    }
  }
  return 0;
}
bool MQTT_is_SWgroup_topic(char *_topic)
{
  for (uint8_t i = 0; i < sizeof(SwGroupTopics) / sizeof(SwGroupTopics[0]); i++) /* SENT FOR A GROUP TOPIC */
  {
    if (strcmp(_topic, SwGroupTopics[i]) == 0)
    {
      return 1;
    }
  }
  return 0;
}

/* Answer Status CMD */
void MQTT_Post_Win_status(uint8_t i, char *msg)
{
  sprintf(msg, "Status: Win[#%d][%s] [%s]%s", i, winSW_V[i]->name, winMQTTcmds[winSW_V[i]->get_winState()], i == (swEntityCounter - 1) ? "" : "; ");
  iot.pub_msg(msg);
}
void MQTT_Post_SW_status(uint8_t i, char *msg)
{
  sprintf(msg, "Status: SW[#%d][%s] [%s]", i, SW_v[i]->name, SW_v[i]->get_SWstate() ? "On" : "Off"); //, i == swEntityCounter - 1 ? "" : "; ");
  iot.pub_msg(msg);
}
void MQTT_WinGroup_status(char *msg)
{
  for (uint8_t i = 0; i < winEntityCounter; i++)
  {
    MQTT_Post_Win_status(i, msg);
  }
}
void MQTT_SWGroup_status(char *msg)
{
  for (uint8_t i = 0; i < swEntityCounter; i++)
  {
    MQTT_Post_SW_status(i, msg);
  }
}

/* Switch Windows CMDs*/
void MQTT_clear_retained(char *topic)
{
  // iot.pub_noTopic("", topic, true);
  yield();
}
bool MQTT_is_CHG_winState(char *inmsg, uint8_t &ret_state)
{
  if (strcmp(inmsg, winMQTTcmds[1]) == 0) /* UP */
  {
    ret_state = UP;
    return 1;
  }
  else if (strcmp(inmsg, winMQTTcmds[2]) == 0) /* DOwN */
  {
    ret_state = DOWN;
    return 1;
  }
  else if (strcmp(inmsg, winMQTTcmds[0]) == 0) /* STOP */
  {
    ret_state = STOP;
    return 1;
  }
  else
  {
    return 0;
  }
}
void MQTT_Win_Change_state(uint8_t i, uint8_t state)
{
  winSW_V[i]->set_WINstate(state, MQTT);
#if RETAINED_MSG
  MQTT_clear_retained(winSW_V[i]->name);
#endif
}
void MQTT_WinGroup_Change_state(uint8_t state)
{
  for (uint8_t i = 0; i < winEntityCounter; i++)
  {
    MQTT_Win_Change_state(i, state);
  }
}

/* Switch Windows CMDs*/
bool MQTT_is_CHG_SWstate(char *inmsg, uint8_t &ret_state)
{
  if (strcmp(inmsg, SW_MQTT_cmds[1]) == 0) /* ON */
  {
    ret_state = 1;
    return 1;
  }
  else if (strcmp(inmsg, SW_MQTT_cmds[0]) == 0) /* OFF */
  {
    ret_state = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}
void MQTT_SW_Change_state(uint8_t i, uint8_t state, unsigned int TO = 0)
{
  if (state == 1) /* ON */
  {
    SW_v[i]->turnON_cb(EXT_0, TO);
  }
  else if (state == 0) /* OFF */
  {
    SW_v[i]->turnOFF_cb(EXT_0);
  }
#if RETAINED_MSG
  MQTT_clear_retained(SW_v[i]->name);
#endif
}
void MQTT_SWGroup_Change_state(uint8_t state)
{
  for (uint8_t i = 0; i < swEntityCounter; i++)
  {
    if (state == 1) /* ON */
    {
      SW_v[i]->turnON_cb(EXT_0);
    }
    else if (state == 0) /* OFF */
    {
      SW_v[i]->turnOFF_cb(EXT_0);
    }
  }
}

void MQTT_Win_entity(uint8_t i, char *msg)
{
  sprintf(msg, "[Entity]: [Win#%d] topic [%s], Virtual-out [%s], 2nd-input [%s]",
          winSW_V[i]->get_id(), winSW_V[i]->name, winSW_V[i]->virtCMD ? "Yes" : "No", winSW_V[i]->useExtSW ? "Yes" : "No");
  iot.pub_msg(msg);
}
void MQTT_SW_entity(uint8_t i, char *msg)
{
  char *w[] = {"None", "On_off SW", "Push Button"};
  sprintf(msg, "[Entity]: [SW#%d] topic [%s], Virtual-out [%s], type [%s], timeout [%s]",
          SW_v[i]->get_id(), SW_v[i]->name, SW_v[i]->is_virtCMD() ? "Yes" : "No", w[SW_v[i]->get_SWtype()], SW_v[i]->useTimeout() ? "Yes" : "No");
  iot.pub_msg(msg);
}

/* Controller CMDS */
void MQTT_to_controller(char *incoming_msg, char *msg)
{
  if (strcmp(incoming_msg, "status") == 0)
  {
    MQTT_WinGroup_status(msg);
    MQTT_SWGroup_status(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "help #2: {Controller : [status, ver2, help2, entities]}, {Windows : [up, down, off]}, {Switch : [on, off]}");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "ver #2:[%s], [%s], [%s]", ver, swEntityCounter > 0 ? SW_v[0]->ver : "", winEntityCounter > 0 ? winSW_V[0]->ver : "");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "entities") == 0)
  {
    for (uint8_t n = 0; n < winEntityCounter; n++)
    {
      MQTT_Win_entity(n, msg);
    }
    for (uint8_t n = 0; n < swEntityCounter; n++)
    {
      MQTT_SW_entity(n, msg);
    }
  }
}
void MQTT_to_lockdown(char *incoming_msg)
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

/* ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±± */

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
  uint8_t n;
  uint8_t state;
  char msg[150];

  if (MQTT_is_Contl_topic(_topic))
  {
    MQTT_to_controller(incoming_msg, msg);
  }
  else if (MQTT_is_lockdown_topic(_topic))
  {
    MQTT_to_lockdown(incoming_msg);
  }
  else if (MQTT_is_Win_topic(_topic, n))
  {
    if (MQTT_is_CHG_winState(incoming_msg, state))
    {
      MQTT_Win_Change_state(n, state);
    }
    else if (strcmp(incoming_msg, "status") == 0)
    {
      MQTT_Post_Win_status(n, msg);
    }
  }
  else if (MQTT_is_SW_topic(_topic, n))
  {
    if (MQTT_is_CHG_SWstate(incoming_msg, state))
    {
      MQTT_SW_Change_state(n, state);
    }
    else if (strcmp(incoming_msg, "status") == 0)
    {
      MQTT_Post_SW_status(n, msg);
    }
    else
    {
      /* Here Timeout CMD */
      if (iot.num_p > 1 && strcmp(iot.inline_param[0], "timeout") == 0)
      {
        MQTT_SW_Change_state(n, 1, atoi(iot.inline_param[1]));
      }
    }
  }
  else if (MQTT_is_Wingroup_topic(_topic))
  {
    if (MQTT_is_CHG_winState(incoming_msg, state))
    {
      MQTT_WinGroup_Change_state(state);
    }
    else if (strcmp(incoming_msg, "status") == 0)
    {
      MQTT_WinGroup_status(msg);
    }
  }
  else if (MQTT_is_SWgroup_topic(_topic))
  {
    if (MQTT_is_CHG_SWstate(incoming_msg, state))
    {
      MQTT_SWGroup_Change_state(state);
    }
    else if (strcmp(incoming_msg, "status") == 0)
    {
      MQTT_SWGroup_status(msg);
    }
    else
    {
      /* Here Timeout CMD */
    }
  }
}
void startIOTservices()
{
  update_Parameters_flash();
  iot.start_services(addiotnalMQTT);
}