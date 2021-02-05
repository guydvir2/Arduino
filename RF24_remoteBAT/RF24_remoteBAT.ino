#include <myRF24.h>
#include <ArduinoJson.h>

/*~~~~~~~~~~~~~~ Select ROLE ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define ROLE 0 // <----- Change this only //
/* 0:Reciever ( ESP8266 also connected to WiFi) */
/*1: Sender ( Pro-Micro with RF24 log range anttenna)*/
#define PRINT_MESSAGES true
#define SLEEP_TIME 60 //sec
#define USE_SLEEP false
#define MEASURE_VOLTAGE true
#include "rf24_defs.h"
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* ~~~~~~~~~~~~~~~~ Sender ~~~~~~~~~~~~~~~*/
#if ROLE == 1
// #include <Ticker.h>
#if MEASURE_VOLTAGE
#include "measureADC.h"
#endif
#include "power.h"
#define USE_IOT 0           /*For devices NOT connected to WiFi*/
#define dev_name "ProMicro" /*8 letters max*/

/* ~~~~~~~~~~~~~~~~ Reciever ~~~~~~~~~~~~~~~*/
#elif ROLE == 0   /* ESP8266*/
#define USE_IOT 1 /*For ESP8266*/
#include "myIOT_def.h"
#define dev_name "iot_Port" /*8 letters max*/
#endif

/* ~~~~~~~ Keys from JSON formatted msg ~~~~~~ */
char *infos[] = {"defs"};
char *cmds[] = {"reset", "MQTT"};
char *questions[] = {"clk", "wakeTime", "tst"};
const char *keys[] = {"from", "m_type", "vBAT", "vSolar", "sent"}; /* up to 3 key/value pairs*/
const char *m_types[] = {"q", "ans", "cmd", "info", "act"};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

char inmsg_buff[250];
const byte delay_read = 200;
int inmsg_counter[5];     /* counts incoming messages only - which are all successes*/
int outmsg_counter[2][5]; /* row 0 counts fails, row 1 counts success sendings*/

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
bool send(const char *msg_type, const char *p0, const char *p1, const char *p2 = "", const char *key[] = keys)
{

  int i = 0;
  char p3[20];
  char outmsg[250];
  sprintf(p3, "#%d/%d", outmsg_counter[1][i], outmsg_counter[0][i] + outmsg_counter[1][i]);
  createMSG_JSON(outmsg, msg_type, p0, p1, p3, key);
  power_periferials(true);

  /* Sending the message */
  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {
    outMSG_counter(msg_type, i, 1);
    if (PRINT_MESSAGES)
    {
      char a[200];
      sprintf(a, "%s[#%d/%d] >> %s", msg_type, outmsg_counter[1][i], outmsg_counter[0][i] + outmsg_counter[1][i], outmsg);
      Serial.println(a);
    }
    power_periferials(LOW);
    return 1;
  }
  else
  {
    outMSG_counter(msg_type, i, 0);
    if (PRINT_MESSAGES)
    {
      char a[200];
      sprintf(a, "FAILED: %s[#%d/%d] >> %s", msg_type, outmsg_counter[0][i], outmsg_counter[0][i] + outmsg_counter[1][i], outmsg);
      Serial.println(a);
    }
    power_periferials(LOW);
    return 0;
  }
}
void intercept_incoming()
{
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    answer_incoming(inmsg_buff);
  }
}

void answer_incoming(char *inmsg)
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
    // handle_INquestion(DOC);
  }

  /* got an answer */
  else if (strcmp(DOC[keys[1]], m_types[1]) == 0)
  {
    // handle_INanswer(DOC);
  }

  /* got cmd */
  else if (strcmp(DOC[keys[1]], m_types[2]) == 0)
  {
    // handle_INcmd(DOC);
  }
  /* got info */
  else if (strcmp(DOC[keys[1]], m_types[3]) == 0)
  {
    handle_INinfo(DOC);
  }
}
void handle_INinfo(JsonDocument &DOC)
{
  inmsg_counter[1]++;
  // if (strcmp(DOC[keys[2]], cmds[0]) == 0) /* mqtt*/
  // {
#if ROLE == 0
  char t[100];
  serializeJson(DOC, t);
  iot.pub_msg(t);
#endif
  // }
}
bool send_voltage_values()
{
  char msg[10];
  char msg2[10];
  float vBAT = 2.2;
  float vSolar = 1.1;
  byte x = 0;
  const byte max_send_retries = 3;
#if ROLE == 1
#if MEASURE_VOLTAGE
  // vBAT = get_Vmeasure(0);
  // vSolar = get_Vmeasure(1);
#endif

  String conv1 = String(vBAT);
  String conv2 = String(vSolar);

  sprintf(msg, "%sv", conv1.c_str());
  sprintf(msg2, "%sv", conv2.c_str());
  while (!send(m_types[3], msg, msg2) && x < max_send_retries)
  {
    x++;
    delay(10);
    Serial.print("Send fail #");
    Serial.println(x);
  }
#endif
}
void exec_after_wakeup()
{
  send_voltage_values();
}

const byte mosfetPin = 6;

void power_periferials(bool state)
{
  digitalWrite(mosfetPin, state);
  if (state)
  {
    delay(50);
    radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  }
}
void setup()
{
#if ROLE == 1
  pinMode(mosfetPin, OUTPUT);
  power_periferials(HIGH);
  Serial.begin(115200);

  // delay(50);
  // Serial.println("Start");

#if MEASURE_VOLTAGE
  startADC();
#endif

#elif ROLE == 0
  radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
#if USE_IOT == 1
  startIOTservices();
#endif
#endif
}
void loop()
{
#if ROLE == 0
  iot.looper();
  intercept_incoming();
#elif ROLE == 1
  sleepit_10sec(SLEEP_TIME);
#endif
}
