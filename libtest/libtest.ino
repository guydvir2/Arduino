#include <myRF24.h>
#include <ArduinoJson.h>

#define ROLE 0 /* 0:Reciever ( ESP8266 also connected to WiFi) 1: Sender ( Pro-Micro with RF24 log range anttenna)*/
#define PRINT_MESSAGES true
#define PRINT_REPORT true
#define REPORT_SEC 60

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
int outmsg_counter[2][5]; /* row 0 counts fails, row 1 counts success sendings*/
int inmsg_counter[5];     /* counts incoming messages only - which are all successes*/

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
void createMSG_JSON(char a[], const char *v0, const char *v1 = "", const char *v2 = "", const char *v3 = "", const char *key[] = keys)
{
  /* 3 key/value pairs */
  if (strcmp(v3, "") != 0)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], dev_name, key[1], v0, key[2], v1, key[3], v2, key[4], v3);
  }

  /* 2 key/value pairs */
  else if (strcmp(v2, "") != 0)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], dev_name, key[1], v0, key[2], v1, key[3], v2);
  }

  /* 1 key/value pairs */
  else if (strcmp(v1, "") != 0)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], dev_name, key[1], v0, key[2], v1);
  }
}
void outMSG_counter(const char *msg_t, int &i, byte state)
{
  while (i < sizeof(m_types) / sizeof(m_types[0]))
  {
    if (strcmp(msg_t, m_types[i]))
    {
      outmsg_counter[state][i]++; /* state can be 0 or 1. row 0 when fail send and row 1 when it succeeds*/
      break;
    }
    i++;
  }
}
bool send(const char *msg_t, const char *p0, const char *p1, const char *counter = "", const char *key[] = keys)
{
  int i = 0;
  char a[350];
  char outmsg[250];
  const char *k3 = "TEST";
  createMSG_JSON(outmsg, msg_t, p0, p1, k3);

  /* Sending the message */
  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {
    outMSG_counter(msg_t, i, 1);
    if (PRINT_MESSAGES)
    {
      sprintf(a, "%s[#%d/%d] >> %s", msg_t, outmsg_counter[1][i], outmsg_counter[0][i] + outmsg_counter[1][i], outmsg);
      Serial.println(a);
    }
    return 1;
  }
  else
  {
    outMSG_counter(msg_t, i, 0);
    if (PRINT_MESSAGES)
    {
      sprintf(a, "FAILED: %s[#%d/%d] >> %s", msg_t, outmsg_counter[0][i], outmsg_counter[0][i] + outmsg_counter[1][i], outmsg);
      Serial.println(a);
    }
    return 0;
  }
}
void qna(char *inmsg)
{
  /* Protocol Send & Recieve */
  /*{ "from" : <SENDER>, "msg_type" : <q/ans/info/cmd>, "k0" : p0, "k1" : p1, "k2" : <TXT2> } */

  StaticJsonDocument<300> DOC;
  deserializeJson(DOC, (const char *)inmsg);
  if (PRINT_MESSAGES)
  {
    Serial.print(DOC[keys[1]].as<const char *>());
    Serial.print(" <<");
    Serial.println(inmsg);
  }

  /* got a question to answer*/
  if (strcmp(DOC[keys[1]], m_types[0]) == 0)
  {
    handle_INquestion(DOC);
  }

  /* got an answer */
  else if (strcmp(DOC[keys[1]], m_types[1]) == 0)
  {
    handle_INanswer(DOC);
  }
}
void handle_INquestion(JsonDocument &DOC)
{
  char pload1[50];
  inmsg_counter[0]++;
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
}
void handle_INanswer(JsonDocument &DOC)
{
  inmsg_counter[1]++;
  if (strcmp(DOC[keys[2]], questions[0]) == 0) /* p0 is clk - ask for time_stamp - IOT only*/
  {
    // Serial.println("Yes not I know what time it is");
    yield;
  }
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
void intercept_incoming()
{
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    qna(inmsg_buff);
  }
}
void report(int sec_to_report = 10)
{
#if PRINT_REPORT
  static unsigned long lastReport = 0;
  if (millis() > lastReport + sec_to_report * 1000UL)
  {
    lastReport = millis();
    Serial.println("\n~~ Outgoing Messages ~~");
    int n = sizeof(m_types) / sizeof(m_types[0]);
    for (int i = 0; i < n; i++)
    {
      Serial.print(m_types[i]);
      Serial.print("\t: ");
      Serial.print("[");
      Serial.print("Fail:");
      Serial.print(outmsg_counter[0][i]);
      Serial.print("/");
      Serial.print("Total:");
      Serial.print(outmsg_counter[1][i] + outmsg_counter[0][i]);
      Serial.println("]");
    }
    Serial.println("\n~~ Incoming Messages ~~");

    for (int i = 0; i < n; i++)
    {
      Serial.print(m_types[i]);
      Serial.print("\t: ");
      Serial.print("[");
      Serial.print("Total:");
      Serial.print(inmsg_counter[i]);
      Serial.println("]");
    }
    Serial.println("~~~~ END ~~~~");
  }
#endif
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
  intercept_incoming();

  // ~~~~~~~ Sending Questions ~~~~~~~~
  if (ROLE == 0)
  {
    // a_timely_q(2123, m_types[0], questions[0]);
    yield;
  }
  else if (ROLE == 1)
  {
    a_timely_q(1500, m_types[0], questions[0]);
  }
  report(REPORT_SEC);
}
