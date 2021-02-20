#include <myRF24.h>
#include <ArduinoJson.h>
#include <Time.h>

/*~~~~~~~~~~~~~~ Select ROLE ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define ROLE 1 // <----- Change this only //
/* 0:Reciever ( ESP8266 also connected to WiFi) */
/*1: Sender ( Pro-Micro with RF24 log range anttenna)*/
#define PRINT_MESSAGES true
#define SLEEP_TIME 10 //sec
#define USE_SLEEP false
#define MEASURE_VOLTAGE true
#include "rf24_defs.h"
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* ~~~~~~~~~~~~~~~~ Sender ~~~~~~~~~~~~~~~*/
#if ROLE == 1
#if MEASURE_VOLTAGE
// #include "measureADC.h"
#endif
#include "power.h"
#define USE_IOT 0           /*For devices NOT connected to WiFi*/
#define dev_name "ProMicro" /*8 letters max*/
#define V_REF 5.01

/* ~~~~~~~~~~~~~~~~ Reciever ~~~~~~~~~~~~~~~*/
#elif ROLE == 0   /* ESP8266*/
#define USE_IOT 1 /*For ESP8266*/
#include "myIOT_def.h"
#define dev_name "iot_Port" /*8 letters max*/
#endif

/* ~~~~~~~ Keys from JSON formatted msg ~~~~~~ */
char *infos[] = {"defs"};
char *cmds[] = {"reset", "MQTT"};
char *questions[] = {"clk", "wakeTime", "whois_online"};
const char *power_keys[] = {"from", "m_type", "vBAT", "vSolar", "sent"}; /* up to 3 key/value pairs*/
const char *gKeys[] = {"from", "m_type", "key0", "key2", "sent"};        /* up to 3 key/value pairs*/
const char *m_types[] = {"q", "ans", "cmd", "info", "act"};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~ Message buffers and Counters ~~~~~~~~~ */
const int JSON_SIZE = 400;
const byte delay_read = 200;
int inmsg_counter[5];     /* counts incoming messages only - which are all successes*/
int outmsg_counter[2][5]; /* row 0 counts fails, row 1 counts success sendings*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~ Text Manipultings ~~~~~~~~~~~~~~~~~~~~*/
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
void createMSG_JSON(char a[], const char *v0, const char *v1 = nullptr, const char *v2 = nullptr, const char *v3 = nullptr, const char *v4 = nullptr, const char *key[] = gKeys)
{
  /* 3 key/value pairs */
  if (v4 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1, key[2], v2, key[3], v3, key[4], v4);
  }
  else if (v3 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1, key[2], v2, key[3], v3);
  }
  else if (v2 != nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1, key[2], v2);
  }
  else if (strcmp(v1, nullptr) != 0)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1);
  }
  else if (strcmp(v1, nullptr) == 0)
  {
    sprintf(a, "{\"%s\":\"%s\"}", key[0], v0);
  }
}
char *retClk()
{
  char *t = new char;
  sprintf(t, "%d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
  return t;
}

/* ~~~~~~~~~~ Asking & Sending ~~~~~~~~~~*/
bool gen_send(const char *key0, const char *value0, const char *key1 = nullptr, const char *value1 = nullptr, const char *key2 = nullptr,
              const char *value2 = nullptr, const char *key3 = nullptr, const char *value3 = nullptr, const char *key4 = nullptr, const char *value4 = nullptr)
{
  /* 4 pairs of key/values */
  char outmsg[250];
  const char *k[] = {key0, key1, key2, key3, key4};
  createMSG_JSON(outmsg, value0, value1, value2, value3, value4, k);

  /* Sending the message */
  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {
    if (PRINT_MESSAGES)
    {
      Serial.print(">>");
      Serial.println(outmsg);
    }
    return 1;
  }
  else
  {
    if (PRINT_MESSAGES)
    {
      Serial.print("XX");
      Serial.println(outmsg);
    }
    return 0;
  }
}
bool send(const char *msg_type, const char *p0, const char *p1 = nullptr, const char *p2 = nullptr, const char *key[] = gKeys)
{
  int i = 0;
  char p3[20];
  char outmsg[250];
  sprintf(p3, "#%d/%d", outmsg_counter[1][i], outmsg_counter[0][i] + outmsg_counter[1][i]);

  /* Sending the message */
  // power_periferials(true);
  if (gen_send(key[0], dev_name, key[1], msg_type, key[2], p0, key[3], p1, key[4], p2))
  {
    outMSG_counter(msg_type, i, 1);
    // power_periferials(LOW);
    return 1;
  }
  else
  {
    outMSG_counter(msg_type, i, 0);
    // power_periferials(LOW);
    return 0;
  }
}
bool ask4Clk()
{
  if (year() == 1970)
  {
    byte x = 0;
    const byte max_retries = 3;
    while (x < max_retries && !send(m_types[0], questions[0]))
    {
      x++;
      delay(10);
      Serial.println("fail");
    }
    if (x >= max_retries)
    {
      Serial.println("CLK_REQ_FAIL");
      return 0;
    }
    else
    {
      Serial.println("CLK_REQ_OK");
      return 1;
    }
  }
  else
  {
    return 1;
  }
}

/* ~~~~~~~~~~ Incoming Messages ~~~~~~~~~ */
void intercept_incoming()
{
  char inmsg_buff[250];
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    answer_incoming(inmsg_buff);
  }
}
void answer_incoming(char *inmsg)
{
  StaticJsonDocument<JSON_SIZE> DOC;
  deserializeJson(DOC, (const char *)inmsg);

  serializeJsonPretty(DOC, Serial);

  /* got a question to answer.
     gKeys[1]  - is msg type
  */
  if (DOC.containsKey(gKeys[1]))
  {
    if (PRINT_MESSAGES)
    {
      Serial.print("<<");
      Serial.println(inmsg);
    }
    /* got question */
    if (strcmp(DOC[gKeys[1]], m_types[0]) == 0)
    {
      handle_INquestion(DOC);
    }
    /* got an answer */
    else if (strcmp(DOC[gKeys[1]], m_types[1]) == 0)
    {
      handle_INanswer(DOC);
    }
    /* got cmd */
    else if (strcmp(DOC[gKeys[1]], m_types[2]) == 0)
    {
      // handle_INcmd(DOC);
    }
    /* got info */
    else if (strcmp(DOC[gKeys[1]], m_types[3]) == 0)
    {
      handle_INinfo(DOC);
    }
  }
  else
  {
    serializeJsonPretty(DOC, Serial);
  }
}
void handle_INinfo(JsonDocument &DOC)
{
  inmsg_counter[3]++;
#if ROLE == 0
  char t[100];
  serializeJson(DOC, t);
  iot.pub_msg(t);
#endif
}
void handle_INquestion(JsonDocument &DOC)
{
  inmsg_counter[0]++;
  if (strcmp(DOC[gKeys[2]], questions[2]) == 0) /* Who is on line */
  {
    send(m_types[1], questions[2], ":)");
  }
#if ROLE == 0
  if (strcmp(DOC[gKeys[2]], questions[0]) == 0) /* send answer clk */
  {
    char t[20];
    sprintf(t, "%d", now());
    send(m_types[1], questions[0], t);
  }
#endif
}
void handle_INanswer(JsonDocument &DOC)
{
  inmsg_counter[1]++;
  if (strcmp(DOC[gKeys[2]], questions[0]) == 0) /* get clk answer*/
  {
    const char *a = DOC[gKeys[3]];
    setTime(atol(a));
    Serial.print("clk update: ");
    Serial.println(retClk());
  }
}

#if ROLE == 1
float measure_batV(byte x = 3)
{
  float a = 0;
  for (int i = 0; i < x; i++)
  {
    a += analogRead(A0);
  }
  a /= x;
  a /= (float)1023;
  a *= V_REF;
  return a;
}
bool send_voltage_values()
{
  char msg[10];
  char msg2[10];
  float vBAT = measure_batV();
  float vSolar = 1.1;
  byte x = 0;
  const byte max_send_retries = 3;
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
}

void exec_after_wakeup()
{
  send_voltage_values();
}
#endif
const byte mosfetPin = 6;

void power_periferials(bool state)
{
  if (state)
  {
    radio.radio.powerUp();
    delay(50);
    digitalWrite(mosfetPin, state);
    radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  }
  else
  {
    radio.radio.powerDown();
    delay(50);
    digitalWrite(mosfetPin, state);
  }
}

void sender_setup()
{
#if ROLE == 1
  Serial.begin(115200);
  // pinMode(mosfetPin, OUTPUT);
  // power_periferials(HIGH);
  // send(m_types[3], "boot");
#endif
}
void reciever_setup()
{
#if ROLE == 0
  // radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
#if USE_IOT == 1
  startIOTservices();
#else
  Serial.begin(115200);
#endif
#endif
}
void setup()
{
  radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  delay(50);
  sender_setup();
  reciever_setup();
  send(m_types[0], questions[2]); /* Ask whois on-line */
}
void sender_loop()
{
#if ROLE == 1
  static long last = 0;
  if (millis() - last > 3000)
  {
    last = millis();
    send(m_types[0], "bye?");
    // if (!ask4Clk())
    // {
    //   // sleepit_10sec(SLEEP_TIME);
    //   // Serial.println(retClk());
    //   Serial.println("Clock_Err");
    // }
    // else
    // {
    //   Serial.println(retClk());
    //   Serial.println("Goto sleep");
    //   // sleepit_10sec(SLEEP_TIME);
    // }
  }
#endif
}
void reciever_loop()
{
#if USE_IOT == 1 && ROLE == 0
  iot.looper();
  static long last = 0;
  if (millis() - last > 3000)
  {
    last = millis();
    // send(m_types[0], "guy");
    // if (!ask4Clk())
    // {
    //   // sleepit_10sec(SLEEP_TIME);
    //   Serial.println(retClk());
    //   Serial.println("Clock_Err");
    // }
    // else
    // {
    //   Serial.println("Goto sleep");
    //   // sleepit_10sec(SLEEP_TIME);
    // }
  }
#endif
}
void loop()
{
  // intercept_incoming();
  sender_loop();
  reciever_loop();
}
