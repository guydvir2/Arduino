#include <myIOT2.h>
#include <myTelegServer.h>

// ********** Sketch Services  ***********
#define USE_MSG_BOT 1
#define USE_LOG_BOT 1
#define USE_LISTEN_BOT 0
#define time_check_messages 5
#define VER "telegramServer_v0.3"

char LOG_TOPIC[] = "myHome/log";
char MSG_TOPIC[] = "myHome/Messages";

const char cmd_1[] PROGMEM = "/status_all";
const char cmd_2[] PROGMEM = "/windows_up";
const char cmd_3[] PROGMEM = "/windows_down";
const char cmd_4[] PROGMEM = "/windows_stop";
const char cmd_5[] PROGMEM = "/armed_away";
const char cmd_6[] PROGMEM = "/disarmed";
const char cmd_7[] PROGMEM = "/saloon_up";
const char cmd_8[] PROGMEM = "/saloon_down";
const char cmd_9[] PROGMEM = "/help";
const char *const command_set[] PROGMEM = {cmd_1, cmd_2, cmd_3, cmd_4, cmd_5, cmd_6, cmd_7, cmd_8, cmd_9};

const int bufsize = 700; // Max Buffer ~1200 chars long

myIOT2 iot;
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

#define MSG_BOT BOT1
#define LOG_BOT BOT1
#define LISTEN_BOT BOT1

#if USE_MSG_BOT
String MSG_buf = "";
unsigned long MSG_clk = 0;
#endif

#if USE_LOG_BOT
String LOG_buf = "";
unsigned long LOG_clk = 0;
#endif

#if USE_LISTEN_BOT
#endif

// #ifdef BOT1
myTelegram BOT1(BOT_TOKEN, time_check_messages);
// #endif
// myTelegram BOT2(BOT_TOKEN_2, time_check_messages);
// myTelegram BOT3(BOT_TOKEN_3, time_check_messages);

void telecmds(String &in_msg, String &from, String &chat_id, String &snd_msg)
{
  // const char *command_set[] = {"/status_all", "/windows_up", "/windows_down", "/windows_stop", "/armed_away", "/disarmed", "/saloon_up", "/saloon_down", "/help"};

  uint8_t num_commands = 9;

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
      char buffer[20];
      // strcpy_P(buffer, (char *)pgm_read_word(&(command_set[t]))); // Necessary casts and dereferencing, just copy.
      Serial.println(buffer);
      // snd_msg += String(buffer) + String("\n");
    }
  }
}
void MQTT_to_telegramBOTS()
{
  const uint8_t timeout = 3; // sec

  if (iot.extTopic_newmsg_flag)
  {
#if USE_LOG_BOT
    if (strcmp(incoming_mqtt.from_topic, LOG_TOPIC) == 0)
    {
      LOG_clk = millis();
      LOG_buf += String('\n') + String(incoming_mqtt.msg) + String('\n');
      iot.clear_ExtTopicbuff();
    }
#endif
#if USE_MSG_BOT
    if (strcmp(incoming_mqtt.from_topic, MSG_TOPIC) == 0)
    {
      MSG_clk = millis();
      MSG_buf += String('\n') + String(incoming_mqtt.msg) + String('\n');
      iot.clear_ExtTopicbuff();
    }
#endif
  }

#if USE_MSG_BOT
  if (MSG_clk != 0 && ((millis() - MSG_clk) > timeout * 1000 || MSG_buf.length() > bufsize))
  {
    MSG_BOT.send_msg(MSG_buf);
    Serial.print("size:");
    Serial.println(MSG_buf.length());
    MSG_clk = 0;
    MSG_buf = "";
  }

#endif

#if USE_LOG_BOT
  if (LOG_clk != 0 && ((millis() - LOG_clk) > timeout * 1000 || LOG_buf.length() > bufsize))
  {
    LOG_BOT.send_msg(LOG_buf);
    LOG_clk = 0;
    LOG_buf = "";
  }
#endif
}
void startTelegramBOTS()
{
#if USE_LOG_BOT
  LOG_BOT.begin();
  strcpy(LOG_BOT.chatID, CHAT_ID);
#endif

#if USE_MSG_BOT
  MSG_BOT.begin();
  strcpy(MSG_BOT.chatID, CHAT_ID);
#endif

#if USE_LISTEN_BOT
  LISTEN_BOT.begin(telecmds);
  strcpy(LISTEN_BOT.chatID, CHAT_ID);

  char clk[25];
  char msg[50];
  iot.get_timeStamp(clk);
  sprintf(msg, "Telegram Boot %s", clk);
  LISTEN_BOT.send_msg(msg);
#endif
}

void HeapCheck()
{
  static unsigned int freeHeap = 0;
  unsigned int tHeap = ESP.getFreeHeap();
  if (tHeap != freeHeap)
  {
    Serial.print(F("Last Heap:\t"));
    Serial.println((float)freeHeap / 1000, 2);
    Serial.print(F("Current Heap:\t"));
    Serial.println((float)tHeap / 1000, 2);
    freeHeap = tHeap;
    Serial.println(F("~~~~~~~~~~~~~~~~~~~~"));
  }
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
  LISTEN_BOT.looper();
#endif
  MQTT_to_telegramBOTS();
  delay(50);
  HeapCheck();
}
