#include "myIOTesp32.h"

// ±±±±±±±± Main ±±±±±±±±±±±±
myIOT32::myIOT32(char *devTopic, char *ssid, char *wifi_p, char *mqtt_broker, char *mqttU, char *mqttP, int port)
    : mqttClient(espClient)
{
  _mqtt_server = mqtt_broker;
  _user = mqttU;
  _passw = mqttP;
  _wifi_ssid = ssid;
  _wifi_pass = wifi_p;
  _mqtt_port = port;
  strcpy(_deviceName, devTopic);
}
void myIOT32::start()
{
  if (useSerial)
  {
    Serial.begin(9600);
  }
  if (startWifi())
  {
    startMQTT();
  }
  if (useOTA)
  {
    _startOTA();
  }
  if (useWDT)
  {
    _startWDT();
  }
}
void myIOT32::looper()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // Wifi OK
    MQTTloop() ? _networkflags(1) : _networkflags(0);
  }
  else
  {
    // NoWifi
    static long lastWifi_try = 0;
    long now = millis();

    if (millis() - lastWifi_try > RECON_WIFI * 1000 * 60UL)
    {
      lastWifi_try = now;
      startWifi();
    }
  }

  if (millis() - _networkerr_clock > NO_NETWORK_RESET * 60 * 1000UL && _networkerr_clock > 0)
  {
    sendReset("NetWork fail reset");
  }
  // Restart due to alternative MQTT, 30 min
  if (_alternativeMQTTserver && millis() > 1000 * 60 * 30UL)
  {
    sendReset("Alternative MQTT Server");
  }
  // OTA
  if (useOTA && millis() - allowOTA_clock < 1000 * 60UL * _OTA_upload_interval)
  {
    // wait for OTA
    _OTAlooper();
  }
  if (useWDT)
  {
    // start WatchDog
    _feedTheDog();
  }

  static long unsigned lastUpdate = 0;
  if (millis() - lastUpdate > UPDATE_STATUS_MINS * 1000 * 60 || lastUpdate == 0)
  {
    lastUpdate = millis();
    _createStatusJSON();
  }
}

// ±±±±±±±±±MQTT ±±±±±±±±±±±±±
bool myIOT32::_selectMQTTserver()
{
  char *mqttServers[] = {_mqtt_server, MQTT_SERVER1, MQTT_SERVER2, MQTT_SERVER3};
  char *tempserver = "EMPLY SERVER_AAAAA";
  int i = 0;

  while (i < 4)
  {
    i++;
    Serial.println(i);
    Serial.print(mqttServers[i]);
    Serial.print(" :");
    Serial.println(Ping.ping(mqttServers[i]));
  }
  if (i < 4)
  {
    _mqtt_server = mqttServers[i];
    Serial.print("MQTT SERVER: ");
    Serial.println(_mqtt_server);
    if (i != 0)
    {
      _alternativeMQTTserver = true;
    }
    return 1;
  }
  else
  {
    return 0;
  }
}
void myIOT32::MQTTcallback(char *topic, byte *payload, unsigned int length)
{
  char incoming_msg[150];
  char msg[100];

  if (useSerial)
  {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  }

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    incoming_msg[i] = (char)payload[i];
  }
  incoming_msg[length] = 0;
  Serial.println();

  if (strcmp(incoming_msg, "boot") == 0)
  {
    sprintf(msg, "Boot:[%d]", DeviceStatus.boot_clock);
    pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ip") == 0)
  {
    char buf[16];
    sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    sprintf(msg, "IP address:[%s]", buf);
    pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ota") == 0)
  {
    sprintf(msg, "OTA allowed for %d minutes", _OTA_upload_interval);
    pub_msg(msg);
    allowOTA_clock = millis();
  }
  else if (strcmp(incoming_msg, "reset") == 0)
  {
    sendReset("MQTT");
  }
  else
  {
    // ext_mqtt_cb(incoming_msg);
  }

  // if (strcmp(topic, _wakeTopic) == 0)
  // {
  //   strcpy(_incmoing_wakeMSG, incoming_msg);
  // }
  // else if (strcmp(topic, _availTopic) == 0 && bootType == 2 && useResetKeeper)
  // {
  //   // bootType: (2) - value at init , (1) quick boot (0) - regulatBoot
  //   if (strcmp(incoming_msg, "online") == 0)
  //   {
  //     bootType = 1;
  //   }
  //   else if (strcmp(incoming_msg, "offline") == 0)
  //   {
  //     bootType = 0;
  //   }
  //   _notifyOnline();
  // }
  // else if (strcmp(topic, _statusTopic) == 0)
  // {
  //   _getMQTT2JSON(incoming_msg);
  // }
}
void myIOT32::createTopics()
{
  snprintf(_msgTopic, MaxTopicLength, "%s/Messages", prefixTopic);
  snprintf(_groupTopic, MaxTopicLength, "%s/All", prefixTopic);
  snprintf(_errorTopic, MaxTopicLength, "%s/log", prefixTopic);

  if (strcmp(addGroupTopic, "") != 0)
  {
    char temptopic[MaxTopicLength];
    strcpy(temptopic, addGroupTopic);
    snprintf(addGroupTopic, MaxTopicLength, "%s/%s", prefixTopic, temptopic);
    snprintf(deviceTopic, MaxTopicLength, "%s/%s", addGroupTopic, _deviceName);
  }
  else
  {
    snprintf(deviceTopic, MaxTopicLength, "%s/%s", prefixTopic, _deviceName);
  }
  snprintf(_stateTopic, MaxTopicLength, "%s/State", deviceTopic);
  snprintf(_availTopic, MaxTopicLength, "%s/Avail", deviceTopic);
  snprintf(_statusTopic, MaxTopicLength, "%s/Status", deviceTopic);
  snprintf(_wakeTopic, MaxTopicLength, "%s/onWake", deviceTopic);

  // if (useTelegram)
  // {
  //         snprintf(_telegramServer, MaxTopicLength, "%s/%s", prefixTopic, telegramServer);
  // }
}
bool myIOT32::connectMQTT()
{
  if (!mqttClient.connected())
  {
    Serial.println("RECONNECT");
    bool a = mqttClient.connect(_devTopic, _user, _passw, _availTopic, 0, true, "offline");
    networkOK = true;
    _networkerr_clock = 0;
    if (!useResetKeeper)
    {
      _notifyOnline();
    }
    return a;
  }
  else
  {
    return 1;
  }
}
void myIOT32::subscribeMQTT()
{
  for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++)
  {
    if (strcmp(topicArry[i], "") != 0)
    {
      mqttClient.subscribe(topicArry[i]);
      Serial.print("Topic subsribed: ");
      Serial.println(topicArry[i]);
    }
  }
}
bool myIOT32::startMQTT()
{
  mqttClient.setServer(_mqtt_server, _mqtt_port);
  mqttClient.setCallback(std::bind(&myIOT32::MQTTcallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  if (useSerial)
  {
    Serial.print("connect to MQTT Broker: ");
    Serial.println(_mqtt_server);
  }
  createTopics();
  if (connectMQTT())
  {
    subscribeMQTT();
    if (_alternativeMQTTserver)
    {
      char a[50];
      sprintf(a, "<< Boot - alternative MQTT broker: %s", _mqtt_server);
      pub_log(a);
    }
    else
    {
      pub_log("<< Boot >>");
    }
  }
  else
  {
    networkOK = false;
  }
}
bool myIOT32::MQTTloop()
{
  static long lastReconnectAttempt = 0;

  if (mqttClient.connected())
  {
    mqttClient.loop();
    return 1;
  }
  else
  {
    long now = millis();
    Serial.println("MQTT Server- NOT connected");

    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      if (connectMQTT())
      {
        subscribeMQTT();
        _updateKeepAlive();
        _networkflags(1);
        Serial.println("RECONNECT_TRY OK");
        return 1;
      }
      else
      {
        _networkflags(0);
        Serial.println("RECONNECT_TRY FAILED");
        return 0;
      }
    }
    else
    {
      Serial.println("WAIT FOR YOU LOOP");
    }
  }
}
void myIOT32::_notifyOnline()
{
  mqttClient.publish(_availTopic, "online", true);
}
void myIOT32::pub_msg(char *msg)
{
  char tstamp[25];
  char tem[150];
  getTimeStamp(tstamp);
  sprintf(tem, "[%s] [%s] %s", tstamp, deviceTopic, msg);
  mqttClient.publish(_msgTopic, tem);
}
void myIOT32::pub_Status(char *statusmsg)
{
  mqttClient.publish(_statusTopic, statusmsg, true);
}
void myIOT32::pub_nextWake(char *inmsg)
{
  mqttClient.publish(_wakeTopic, inmsg, true);
}
void myIOT32::pub_log(char *inmsg)
{
  char tstamp[25];
  char tem[150];
  getTimeStamp(tstamp);
  sprintf(tem, "[%s] [%s] %s", tstamp, deviceTopic, inmsg);
  mqttClient.publish(_errorTopic, tem);
}

// ±±±±±±±±±±± WIFI & Clock ±±±±±±±±±
void myIOT32::startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 3600, const char *ntpServer = "pool.ntp.org")
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
bool myIOT32::startWifi()
{
  long beginwifi = millis();
  int sec_to_connect = 15;
  WiFi.begin(_wifi_ssid, _wifi_pass);

  while (WiFi.status() != WL_CONNECTED && millis() - beginwifi < sec_to_connect * 1000)
  {
    delay(200);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    sprintf(DeviceStatus.ip, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    startNTP();
    getTime();
    DeviceStatus.boot_clock = _epoch_time;
    _networkflags(1);
    return 1;
  }
  else
  {
    Serial.println("NO-WiFi");
    _networkflags(0);
    return 0;
  }
}
void myIOT32::getTime()
{
  int a = 0;
  while (a < 3)
  {
    if (getLocalTime(&_timeinfo))
    {
      delay(100);
      time(&_epoch_time);
    }
    a++;
  }
}
void myIOT32::getTimeStamp(char ret_timeStamp[25])
{
  getTime();
  sprintf(ret_timeStamp, "%04d-%02d-%02d %02d:%02d:%02d", _timeinfo.tm_year + 1900, _timeinfo.tm_mon, _timeinfo.tm_mday,
          _timeinfo.tm_hour, _timeinfo.tm_min, _timeinfo.tm_sec);
}

// ±±±±±±±±±±±±± OTA & WDT ±±±±±±±±±±±
void myIOT32::_startOTA()
{
  char OTAname[100];
  int m = 0;
  // create OTAname from deviceTopic
  for (int i = ((String)deviceTopic).lastIndexOf("/") + 1; i < strlen(deviceTopic); i++)
  {
    OTAname[m] = deviceTopic[i];
    OTAname[m + 1] = '\0';
    m++;
  }

  allowOTA_clock = millis();

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(OTAname);

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
  if (useSerial)
  {
    Serial.println("OTA - Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}
void myIOT32::_OTAlooper()
{
  ArduinoOTA.handle();
}
void myIOT32::_feedTheDog()
{
  _wdtResetCounter++;
  if (_wdtResetCounter >= _wdtMaxRetries)
  {
    // sendReset("Dog goes woof");
  }
}
void myIOT32::_startWDT()
{
  // wdt.attach(1, std::bind(&myIOT32::_feedTheDog, this)); // Start WatchDog
}
void myIOT32::sendReset(char *header)
{
  char temp[150];

  sprintf(temp, "[%s] - Reset sent", header);

  if (useSerial)
  {
    Serial.println(temp);
  }
  if (strcmp(header, "null") != 0)
  {
    pub_msg(temp);
  }
  delay(1000);
  ESP.restart();
}

// ±±±±±±±± Update JSON status ±±±±±±±±±
void myIOT32::_createStatusJSON()
{
  _updateKeepAlive();

  StaticJsonDocument<JDOC_SIZE> doc;
  doc["topic"] = DeviceStatus.devicetopic; // minutes
  doc["ip"] = DeviceStatus.ip;
  doc["boot"] = DeviceStatus.boot_clock; // minutes
  doc["imAlive"] = DeviceStatus.last_keepalive;
  doc["in1"] = DeviceStatus.input1;
  doc["in2"] = DeviceStatus.input2;
  doc["out1"] = DeviceStatus.output1;
  doc["out2"] = DeviceStatus.output2;

  String output;
  serializeJson(doc, output);
  char a[250];
  output.toCharArray(a, 250);
  pub_Status(a);
}
void myIOT32::createWakeJSON()
{
  StaticJsonDocument<JDOC_SIZE> doc;
  doc["wakeCmd"] = DeviceStatus.wake_cmd;
  doc["nextWake"] = DeviceStatus.nextWake;
  doc["sleepTime"] = DeviceStatus.sleeptime; // minutes
  doc["isWake"] = DeviceStatus.wake_status;

  String output;
  serializeJson(doc, output);
  char a[250];
  output.toCharArray(a, 250);
  pub_nextWake(a);
  Serial.println(output);
}
void myIOT32::_getMQTT2JSON(char *input_str)
{
  StaticJsonDocument<JDOC_SIZE> doc;
  DeserializationError error = deserializeJson(doc, input_str);

  if (error && useSerial)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  const char *sensor = doc["sensor"];
  long time = doc["nextWake"];
}
void myIOT32::_updateKeepAlive()
{
  getTime();
  DeviceStatus.last_keepalive = _epoch_time;
}
void myIOT32::_networkflags(bool s)
{
  if (s == false)
  {
    if (_networkerr_clock == 0 || networkOK == true)
    {
      _networkerr_clock = millis();
      networkOK = false;
      Serial.println("BAD NETW");
    }
  }
  else
  {
    if (_networkerr_clock > 0 || networkOK == false)
    {
      _networkerr_clock = 0;
      networkOK = true;
      Serial.println("GOOD NETW");
    }
  }
}

// ±±±±±±±±±±±±± Others ±±±±±±±±±±±±
bool myIOT32::checkInternet(char *externalSite, byte pings)
{
  return Ping.ping(externalSite, pings);
}
int myIOT32::_inline_read(char *inputstr)
{
  char *pch;
  int i = 0;

  pch = strtok(inputstr, " ,.-");
  while (pch != NULL)
  {
    sprintf(inline_param[i], "%s", pch);
    pch = strtok(NULL, " ,.-");
    i++;
  }
  return i;
}