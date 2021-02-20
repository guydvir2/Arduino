#include <myRF24.h>
#include <ArduinoJson.h>

#define ROLE 1 /* 0:Reciever ( ESP8266 also connected to WiFi) 1: Sender ( Pro-Micro with RF24 log range anttenna)*/
#include "rf24_defs.h"
#define PRINT_MESSAGES true

#if ROLE == 1
const char *dev_name = "arduino";
const char *static_msg = "hi_from_arduino";
const int time_resend = 1980;

#elif ROLE == 0
const char *dev_name = "Wemos";
const char *static_msg = "hi_wemos";
const int time_resend = 4567;

#endif

const char *gKeys[] = {"from", "m_type", "key0", "key2", "sent"}; /* up to 3 key/value pairs*/
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
  else if (v1!= nullptr)
  {
    sprintf(a, "{\"%s\":\"%s\",\"%s\":\"%s\"}", key[0], v0, key[1], v1);
  }
  else if (strcmp(v1, nullptr) == 0)
  {
    sprintf(a, "{\"%s\":\"%s\"}", key[0], v0);
  }
}
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
      Serial.print(">>[OK] ");
      Serial.println(outmsg);
    }
    return 1;
  }
  else
  {
    if (PRINT_MESSAGES)
    {
      Serial.print(">>[F] ");
      Serial.println(outmsg);
    }
    return 0;
  }
}

void send_msg()
{
  static long last_msg = 0;
  while (millis() - last_msg > time_resend)
  {
    last_msg = millis();
    gen_send("a0", dev_name, "a1", "v2");
  }
}
void recv_msg()
{
  char inmsg_buff[250];
  const int delay_read = 100;
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    Serial.print("<<[IN] ");
    Serial.println(inmsg_buff);
  }
}
void setup()
{
  Serial.begin(115200);
  Serial.println("Start!!!");
  radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
}

void loop()
{
  send_msg();
  recv_msg();
}
