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
    // getTime();
    startMQTT();
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
  // if (millis() - _networkerr_clock > 60000)
  // {
  //   ESP.restart();
  // }
}

void myIOT32::MQTTcallback(char *topic, byte *payload, unsigned int length)
{
  char incoming_msg[150];
  char msg[100];

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    incoming_msg[i] = (char)payload[i];
  }
  incoming_msg[length] = 0;
  Serial.println();
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
void myIOT32::mqtt_pubmsg(char *msg)
{
  mqttClient.publish(_msgTopic, msg);
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
  mqttClient.publish(_availTopic, "online");
}
void myIOT32::startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 0, const char *ntpServer = "pool.ntp.org")
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
      //     delay(100);
      //     time(&_epoch_time);
    }
    a++;
  }
}
