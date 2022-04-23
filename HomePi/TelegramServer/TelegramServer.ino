#include <myIOT2.h>
#include <myTelegServer.h>

// ********** Sketch Services  ***********
#define USE_MSG_BOT 0
#define USE_LOG_BOT 0
#define USE_LISTEN_BOT 1
#define time_check_messages 10
#define VER "telegramSever_v0.3"

char *LOG_TOPIC = "myHome/log";
char *MSG_TOPIC = "myHome/Messages";

myIOT2 iot;
#if USE_MSG_BOT
myTelegram MQTT_MSG_TELEGRAM(BOT_TOKEN, time_check_messages);
#endif
#if USE_LOG_BOT
myTelegram MQTT_LOG_TELEGRAM(BOT_TOKEN_2, time_check_messages);
#endif
#if USE_LISTEN_BOT
myTelegram LISTEN_TO_TELEGRAM(BOT_TOKEN_3, time_check_messages);
#endif

MQTT_msg incoming_mqtt;

void startIOTservices()
{
  iot.useFlashP = true;
  if (USE_MSG_BOT)
  {
    iot.extTopic[0] = MSG_TOPIC;
  }
  if (USE_LOG_BOT)
  {
    iot.extTopic[1] = LOG_TOPIC;
  }
  iot.extTopic_msgArray[0] = &incoming_mqtt;
  iot.start_services(addiotnalMQTT);
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: OK");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help2: Commands #3 - [NEW]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "Ver2: Ver:%s", VER);
    iot.pub_msg(msg);
  }
}
void telecmds(String &in_msg, String &from, String &chat_id, String &snd_msg)
{
  String command_set[] = {"/status_all", "/windows_up", "/windows_down", "/windows_stop",
                          "/armed_away", "/disarmed", "/saloon_up", "/saloon_down", "/help"};

  uint8_t num_commands = sizeof(command_set) / sizeof(command_set[0]);

  if (in_msg == command_set[0])
  {
    iot.pub_noTopic("status", "myHome/All");
  }
  else if (in_msg == command_set[1])
  {
    iot.pub_noTopic("up", "myHome/Windows");
    snd_msg = "Windows [UP]";
  }
  else if (in_msg == command_set[2])
  {
    iot.pub_noTopic("down", "myHome/Windows");
    snd_msg = "Windows [DOWN]";
  }
  else if (in_msg == command_set[3])
  {
    iot.pub_noTopic("stop", "myHome/Windows");
    snd_msg = "Windows [STOP]";
  }
  else if (in_msg == command_set[4])
  {
    iot.pub_noTopic("armed_away", "myHome/alarmMonitor");
    snd_msg = "[Armed_AWAY]";
  }
  else if (in_msg == command_set[5])
  {
    iot.pub_noTopic("disarmed", "myHome/alarmMonitor");
    snd_msg = "[DISARMED]";
  }
  else if (in_msg == command_set[6])
  {
    iot.pub_noTopic("up", "myHome/Windows/saloonExit");
    snd_msg = "[UP]";
  }
  else if (in_msg == command_set[7])
  {
    iot.pub_noTopic("down", "myHome/Windows/saloonExit");
    snd_msg = "[DOWN]";
  }

  else if (in_msg == command_set[num_commands - 1])
  {
    snd_msg = "Available Commands:\n";
    for (uint8_t t = 0; t < num_commands; t++)
    {
      snd_msg += command_set[t] + String("\n");
    }
  }
}
void MQTT_to_telegramBOTS()
{
  const int bufsize = 0.8 * 1024; // Max Buffer ~1200 chars long
  const uint8_t timeout = 3;      // sec
  if (USE_MSG_BOT)
  {
    static unsigned long MSG_clk = 0;
    static String MSG_buf = "";
  }
  if (USE_LOG_BOT)
  {
    static unsigned long LOG_clk = 0;
    static String LOG_buf = "";
  }

  if (iot.extTopic_newmsg_flag)
  {
#if USE_LOG_BOT
    if (strcmp(incoming_mqtt.from_topic, LOG_TOPIC) == 0)
    {
      LOG_clk = millis();
      LOG_buf += String('\n') + String(incoming_mqtt.msg) + String('\n');
    }
#endif
#if USE_MSG_BOT
    if (strcmp(incoming_mqtt.from_topic, MSG_TOPIC) == 0)
    {
      MSG_clk = millis();
      MSG_buf += String('\n') + String(incoming_mqtt.msg) + String('\n');
    }
    iot.clear_ExtTopicbuff();
#endif

#if USE_LOG_BOT
    if (LOG_clk != 0 && (millis() - LOG_clk) > timeout * 1000 || LOG_buf.length() > bufsize)
    {
      MQTT_LOG_TELEGRAM.send_msg(LOG_buf);
      LOG_clk = 0;
      LOG_buf = "";
    }
#endif
#if USE_MSG_BOT
    if (MSG_clk != 0 && (millis() - MSG_clk) > timeout * 1000 || MSG_buf.length() > bufsize)
    {
      MQTT_MSG_TELEGRAM.send_msg(MSG_buf);
      Serial.print("size:");
      Serial.println(MSG_buf.length());
      MSG_clk = 0;
      MSG_buf = "";
    }
#endif
  }
}
void startTelegramBOTS()
{

#if USE_LOG_BOT
  MQTT_LOG_TELEGRAM.begin();
  MQTT_LOG_TELEGRAM.chatID = CHAT_ID;
#endif

#if USE_MSG_BOT
  MQTT_MSG_TELEGRAM.begin();
  MQTT_MSG_TELEGRAM.chatID = CHAT_ID;
#endif
#if USE_LISTEN_BOT
  LISTEN_TO_TELEGRAM.begin(telecmds);
  strcpy(LISTEN_TO_TELEGRAM.chatID, CHAT_ID);
#endif
  char clk[25];
  char msg[50];
  iot.get_timeStamp(clk);
  sprintf(msg, "Telegram Boot %s", clk);

#if USE_LISTEN_BOT
  LISTEN_TO_TELEGRAM.send_msg(msg);
#endif
}

void setup()
{
  startIOTservices();
  startTelegramBOTS();
}
void loop()
{
  iot.looper();
#if USE_LISTEN_BOT
  LISTEN_TO_TELEGRAM.looper();
#endif
  // MQTT_to_telegramBOTS();
}
