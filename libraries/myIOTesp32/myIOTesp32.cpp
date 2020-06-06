#include "myIOTesp32.h"

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
  if (startWifi())
  {
    startMQTT();
  }
  if (useSerial)
  {
    Serial.begin(9600);
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
    if (!MQTTloop() && _networkerr_clock == 0)
    {
      _networkerr_clock = millis();
    }
    else
    {
      _networkerr_clock = 0;
    }
  }
  else
  {
    startWifi();
  }
  if (millis() - _networkerr_clock > 60000 && _networkerr_clock != 0)
  {
    ESP.restart();
  }
  if (useOTA && millis() - allowOTA_clock < 1000 * 600UL)
  {
    _OTAlooper();
  }
  if (useWDT)
  {
    _feedTheDog();
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

  if (strcmp(topic, _wakeTopic) == 0)
  {
    strcpy(_incmoing_wakeMSG, incoming_msg);
  }
  else if (strcmp(topic, _availTopic) == 0 && bootType == 2)
  {
    // bootType: (2) - value at init , (1) quick boot (0) - regulatBoot
    if (strcmp(incoming_msg, "online") == 0)
    {
      bootType = 1;
    }
    else if (strcmp(incoming_msg, "offline") == 0)
    {
      bootType = 0;
    }
    Serial.print("BootType: ");
    Serial.println(bootType);
  }
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
    bool a = mqttClient.connect(_devTopic, _user, _passw, _availTopic, 0, true, "offline");
    return a;
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
  createTopics();
  if (connectMQTT())
  {
    subscribeMQTT();
    _notifyOnline();
    pub_log("<< Boot >>");
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
      bool a = connectMQTT();
      subscribeMQTT();
      return a;
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
void myIOT32::startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 3600, const char *ntpServer = "pool.ntp.org")
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
bool myIOT32::startWifi()
{
  long beginwifi = millis();
  WiFi.begin(_wifi_ssid, _wifi_pass);

  while (WiFi.status() != WL_CONNECTED && millis() - beginwifi < 30000)
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
    startNTP();
    return 1;
  }
  else
  {
    Serial.println("NO-WiFi");
    _networkerr_clock = millis();
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

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
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
    sendReset("Dog goes woof");
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