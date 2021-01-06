#include <myRF24.h>
#include <ArduinoJson.h>

#define ROLE 1 /* 0:Reciever ( ESP8266 also connected to WiFi) 1: Sender ( Pro-Micro with RF24 log range anttenna)*/
#include "rf24_defs.h"
#include "myIOT_def.h"

// ~~~~~~~~~~~~~~ Cmds and Questions ~~~~~~~~~
/*

Question - sent and answer is expected from other side.
info - Share information ( sensor readings, update parameters) 
Commands - send an execution command ( either side )

*/
char *cmds[] = {"reset", "MQTT"};
char *infos[] = {"defs"};
char *questions[] = {"clk", "wakeTime", "tst"};
int q_counter = 0;
int a_counter = 0;
int info_counter = 0;
int cmd_counter = 0;

void convert_sec2Clock(long timedelta, char ret_str[])
{
  byte days = 0;
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;

  int sec2minutes = 60;
  int sec2hours = (sec2minutes * 60);
  int sec2days = (sec2hours * 24);
  int sec2years = (sec2days * 365);

  days = (int)(timedelta / sec2days);
  hours = (int)((timedelta - days * sec2days) / sec2hours);
  minutes = (int)((timedelta - days * sec2days - hours * sec2hours) / sec2minutes);
  seconds = (int)(timedelta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

  sprintf(ret_str, "%dd %02d:%02d:%02d", days, hours, minutes, seconds);
}
char *create_msg_id(const char *msg_t)
{
  char *msg_T2 = new char[15];

  if (strcmp(msg_t, "ans") != 0)
  {
    if (strcmp(msg_t, "q") == 0)
    {
      q_counter++;
      sprintf(msg_T2, "#q_%d", q_counter);
    }
    else if (strcmp(msg_t, "info") == 0)
    {
      info_counter++;
      sprintf(msg_T2, "#i_%d", info_counter);
    }
    else if (strcmp(msg_t, "cmd") == 0)
    {
      cmd_counter++;
      sprintf(msg_T2, "#c_%d", cmd_counter);
    }
  }
  return msg_T2;
}
bool send(const char *msg_t, const char *p0, const char *p1, const char *counter = "")
{
  char outmsg[250];
  if (strcmp(msg_t, "ans") != 0)
  {
    radio.genJSONmsg(outmsg, msg_t, p0, p1, "msg_id", create_msg_id(msg_t));
  }
  else
  {
    radio.genJSONmsg(outmsg, msg_t, p0, p1, "msg_id", counter); /* gets q_id from q/info/cmd */
  }

  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {
    Serial.print(msg_t);
    Serial.print(" >>");
    Serial.println(outmsg);
    return 1;
  }
  else
  {
    Serial.print(msg_t);
    Serial.print(" failed >>");
    Serial.println(outmsg);
    return 0;
  }
}
void qna(char *inmsg)
{
  /* Protocol Send & Recieve */
  /*{ "from" : <SENDER>, "msg_type" : <q/ans/info/cmd>, "key_0" : <TXT0>, "key_1" : <TXT1>, "key_2" : <TXT2>, "key_3" : <TXT3> } */

  char pload1[50];
  StaticJsonDocument<300> DOC;
  deserializeJson(DOC, (const char *)inmsg);

  Serial.print(DOC["msg_type"].as<const char *>());
  Serial.print(" <<");
  Serial.println(inmsg);

  if (strcmp(DOC["msg_type"], "q") == 0) /* got a question */
  {
    if (strcmp(DOC["key_0"], questions[0]) == 0) /*time_stamp - IOT only*/
    {
#if USE_IOT == 1
      iot.get_timeStamp();
      send("ans", questions[0], iot.timeStamp, DOC["key_3"]);
#else
      convert_sec2Clock((long)millis() / 1000, pload1);
      send("ans", questions[0], pload1, DOC["key_3"]);
#endif
    }
    else if (strcmp(DOC["key_0"], questions[2]) == 0) /*time_stamp - IOT only*/
    {
      send("ans", questions[2], "got_it", DOC["key_3"]);
    }
  }
  else if (strcmp(DOC["msg_type"], "ans") == 0) /* got a reply */
  {
    // Serial.print("got Answer: ");
    // Serial.println(inmsg);
  }
  else if (strcmp(DOC["msg_type"], "info") == 0) /* ask for information */
  {
    if (strcmp(DOC["key_0"], infos[0]) == 0) /*wake time*/
    {
      convert_sec2Clock((long)millis() / 1000, pload1);
      send("info", infos[0], pload1, DOC["key_3"]);
    }
  }
  else if (strcmp(DOC["msg_type"], "cmd") == 0) /* ask for execute command */
  {
    if (strcmp(DOC["key_0"], cmds[0]) == 0) /* commence RESET*/
    {
#if USE_IOT == 1
      iot.sendReset("RF24cmd");
#endif
      send("ans", cmds[0], "executed", DOC["key_3"]);
    }
    else if (strcmp(DOC["key_0"], cmds[1]) == 0) /* send MQTT msg*/
    {
#if ROLE == 0
      char p[100];
      strcpy(p, DOC["key_1"].as<const char *>());
#if USE_IOT == 1
      iot.pub_msg(p);
#endif
#endif
    }
  }
  else
  {
    Serial.print("some error: ");
    Serial.println(inmsg);
  }
}
void a_timely_q(long tint, const char *msg_t, char *p0, char *p1)
{
  static long question_clock = 0;
  if (millis() - question_clock > tint)
  {
    send(msg_t, p0, p1);
    question_clock = millis();
  }
}
void setup()
{
#if USE_IOT == 1
  startIOTservices();
#else
  Serial.begin(115200);
#endif
  radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
}
void loop()
{
#if USE_IOT == 1
  iot.looper();
#endif

  // ~~~~~~~~~ Listen for Questions ~~~~~~~~~~~~
  char inmsg[250];
  if (radio.RFread2(inmsg, 200))
  {
    qna(inmsg);
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // ~~~~~~~ Sending Questions ~~~~~~~~
  if (ROLE == 0)
  {
    a_timely_q(13679, "q", questions[2], "Na");
  }
  else if (ROLE == 1)
  {
    static unsigned long t = 0;
    if (millis() - t > 12495)
    {
      send("q", questions[1], "baaa");
      t = millis();
    }
    //    a_timely_q(7321, "q", questions[0], "timeStamp");
    // a_timely_q(10510, "q", questions[2], "Na");
  }
}
