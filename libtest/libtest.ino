#include <myRF24.h>
#include <ArduinoJson.h>

#define ROLE 0 /* 0:Reciever ( ESP8266 also connected to WiFi) 1: Sender ( Pro-Micro with RF24 log range anttenna)*/

#include "rf24_defs.h"
#if ROLE == 0
#include "myIOT_def.h"
#endif

// ~~~~~~~~~~~~~~ Cmds and Questions ~~~~~~~~~
/*
Question - sent and answer is expected from other side.
info - Share information ( sensor readings, update parameters) 
Commands - send an execution command ( either side )
*/
char inmsg_buff[250];
const byte delay_read = 200;

char *infos[] = {"defs"};
char *cmds[] = {"reset", "MQTT"};
char *questions[] = {"clk", "wakeTime", "tst"};
const char *keys[] = {"from", "m_type", "k0", "k1", "k2"}; /* up to 3 key/value pairs*/
const char *m_types[] = {"q", "ans", "cmd", "info", "act"};
int msg_counter[2][5];

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
  // char *msg_T2 = new char[15];

  // if (strcmp(msg_t, "ans") != 0)
  // {
  //   if (strcmp(msg_t, "q") == 0)
  //   {
  //     q_counter++;
  //     sprintf(msg_T2, "#q_%d", q_counter);
  //   }
  //   else if (strcmp(msg_t, "info") == 0)
  //   {
  //     info_counter++;
  //     sprintf(msg_T2, "#i_%d", info_counter);
  //   }
  //   else if (strcmp(msg_t, "cmd") == 0)
  //   {
  //     cmd_counter++;
  //     sprintf(msg_T2, "#c_%d", cmd_counter);
  //   }
  // }
  // return msg_T2;
}
void genJSON(char a[], const char *v0, const char *v1 = "", const char *v2 = "", const char *key[] = keys)
{
  if (strcmp(v2, "") != 0)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s,\"%s\":\"%s\"}", key[0], dev_name, key[1], v0, key[2], v1, key[3], v2);
  }
  else if (strcmp(v1, "") != 0)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], dev_name, key[1], v0, key[2], v1);
  }
}
bool send(const char *msg_t, const char *p0, const char *p1, const char *counter = "", const char *key[] = keys)
{
  int i = 0;
  char a[350];
  char outmsg[250];
  genJSON(outmsg, msg_t, p0, p1);
  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {  
    while (i < sizeof(m_types) / sizeof(m_types[0]))
    {
      if (strcmp(msg_t, m_types[i]))
      {
        msg_counter[1][i]++;
        break;
      }
      i++;
    }
    sprintf(a, "%s[#%d/%d] >> %s", msg_t, msg_counter[1][i], msg_counter[0][i] + msg_counter[1][i], outmsg);
    Serial.println(a);
    return 1;
  }
  else
  {
    while (i < sizeof(m_types) / sizeof(m_types[0]))
    {
      if (strcmp(msg_t, m_types[i]))
      {
        msg_counter[0][i]++;
        break;
      }
      i++;
    }
    sprintf(a, "FAILED: %s[#%d/%d] >> %s", msg_t, msg_counter[0][i], msg_counter[0][i] + msg_counter[1][i], outmsg);
    Serial.println(a);
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

  Serial.print(DOC[keys[1]].as<const char *>());
  Serial.print(" <<");
  Serial.println(inmsg);

  /* got a question to answer*/
  if (strcmp(DOC[keys[1]], m_types[0]) == 0)
  {
    if (strcmp(DOC[keys[2]], questions[0]) == 0) /* p0 is clk - ask for time_stamp - IOT only*/
    {
#if USE_IOT == 1
      iot.get_timeStamp();
      send(m_types[1], questions[0], iot.timeStamp, DOC[keys[4]]); // send ans for clk
#else
      convert_sec2Clock((long)millis() / 1000, pload1);
      send(m_types[1], questions[0], pload1, DOC[keys[4]]);
#endif
    }
    // else if (strcmp(DOC[keys[2]], questions[2]) == 0) /*P0 is "tst*/
    // {
    //   send(m_types[1], questions[2], "got_it", DOC[keys[4]]);
    // }
  }
  // /* got an answer */
  // else if (strcmp(DOC[keys[1]], m_types[1]) == 0)
  // {
  //   // yield;
  //   Serial.print("got Answer: ");
  //   Serial.println(inmsg);
  // }
  //   else if (strcmp(DOC["msg_type"], "info") == 0) /* ask for information */
  //   {
  //     if (strcmp(DOC["key_0"], infos[0]) == 0) /*wake time*/
  //     {
  //       convert_sec2Clock((long)millis() / 1000, pload1);
  //       send("info", infos[0], pload1, DOC["key_3"]);
  //     }
  //   }
  //   else if (strcmp(DOC["msg_type"], "cmd") == 0) /* ask for execute command */
  //   {
  //     if (strcmp(DOC["key_0"], cmds[0]) == 0) /* commence RESET*/
  //     {
  // #if USE_IOT == 1
  //       iot.sendReset("RF24cmd");
  // #endif
  //       send("ans", cmds[0], "executed", DOC["key_3"]);
  //     }
  //     else if (strcmp(DOC["key_0"], cmds[1]) == 0) /* send MQTT msg*/
  //     {
  // #if ROLE == 0
  //       char p[100];
  //       strcpy(p, DOC["key_1"].as<const char *>());
  // #if USE_IOT == 1
  //       iot.pub_msg(p);
  // #endif
  // #endif
  //     }
  //   }
  //   else
  //   {
  //     Serial.print("some error: ");
  //     Serial.println(inmsg);
  //   }
}

void a_timely_q(long tint, const char *msg_t, char *p0, char *p1 = "")
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
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    qna(inmsg_buff);
  }
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // ~~~~~~~ Sending Questions ~~~~~~~~
  if (ROLE == 0)
  {
    a_timely_q(2123, m_types[0], questions[0]);
  }
  else if (ROLE == 1)
  {
    a_timely_q(1500, m_types[0], questions[0]);
  }
}
