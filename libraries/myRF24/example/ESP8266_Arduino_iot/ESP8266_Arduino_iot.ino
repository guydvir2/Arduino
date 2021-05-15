#include <myRF24.h>
#include <ArduinoJson.h>

/* 0:Reciever ( ESP8266 also connected to WiFi) */
/* 1: Sender ( Arduino with RF24 log range anttenna)*/
#define ROLE 0
#define PRINT_MESSAGES true
#define JSON_SIZE 300

#include "rf24_defs.h"
#include "create_RF_msg.h"

#if ROLE == 1
const char *dev_name = "Arduino";
const int time_resend = 7890;

#elif ROLE == 0   /* ESP8266*/
#define USE_IOT 1 /*For ESP8266*/
#include "myIOT_def.h"
const char *dev_name = "Wemos";
const int time_resend = 3456;
#endif

void answer_incoming(char *inmsg)
{
  StaticJsonDocument<JSON_SIZE> DOC;
  deserializeJson(DOC, (const char *)inmsg);

  // serializeJsonPretty(DOC, Serial);

  /* got a question to answer. gKeys[1]  - is msg type */
  if (DOC.containsKey(gKeys[1]))
  {
    if (PRINT_MESSAGES)
    {
      Serial.print("<<[IN] ");
      Serial.println(inmsg);
    }
    /* got question */
    if (strcmp(DOC[gKeys[1]], m_types[0]) == 0)
    {
      handle_INquestion(DOC);
    }
    // /* got an answer */
    // else if (strcmp(DOC[gKeys[1]], m_types[1]) == 0)
    // {
    //   handle_INanswer(DOC);
    // }
    // /* got cmd */
    // else if (strcmp(DOC[gKeys[1]], m_types[2]) == 0)
    // {
    //   // handle_INcmd(DOC);
    // }
    // /* got info */
    // else if (strcmp(DOC[gKeys[1]], m_types[3]) == 0)
    // {
    //   handle_INinfo(DOC);
    // }
  }
  else
  {
    serializeJsonPretty(DOC, Serial);
  }
}
void handle_INquestion(JsonDocument &DOC)
{
  /* Who is on line */
  if (strcmp(DOC[gKeys[2]], questions[2]) == 0)
  {
    send(m_types[1], questions[2], ":)");
#if ROLE == 0 && USE_IOT == 1
    char a[50];
    sprintf(a, "Boot: %s", DOC[gKeys[0]].as<char *>());
    iot.pub_log(a);
#endif
  }
  /* Ask for Clock/Uptime */
  else if (strcmp(DOC[gKeys[2]], questions[0]) == 0)
  {
    char q[30];
#if ROLE == 1
    unsigned long upTime = millis() / 1000;
    sprintf(q, "%d[sec]", upTime);
#elif ROLE == 0
    iot.get_timeStamp();
    sprintf(q, "%s", iot.timeStamp);
#endif
    send(m_types[1], questions[0], q);
  }
}
void send_timely_msgs()
{
  static long last_msg = 0;
  while (millis() - last_msg > time_resend)
  {
    last_msg = millis();
    send(m_types[0], questions[0]);
  }
}
void recv_msg()
{
  char inmsg_buff[250];
  const int delay_read = 100;
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    answer_incoming(inmsg_buff);
  }
}
void setup_iot()
{
#if ROLE == 0 /* ESP8266 */
  radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
#if USE_IOT == 1
  startIOTservices();
#else
  Serial.begin(115200);
#endif
  send(m_types[0], questions[2]);
#endif
}
void setup_sender()
{
#if ROLE == 1
  radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  Serial.begin(115200);
  send(m_types[0], questions[2]);

#endif
}
void setup()
{
  setup_sender();
  setup_iot();
  Serial.println("Start!");
}

void loop()
{
  recv_msg();
#if ROLE == 0 && USE_IOT == 1
  iot.looper();
#endif
  send_timely_msgs();
}
