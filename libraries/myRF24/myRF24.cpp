#include "myRF24.h"

myRF24::myRF24(int CE_PIN, int CSN_PIN) : radio(CE_PIN, CSN_PIN)
{
}
void myRF24::startRF24(const byte &w_addr, const byte &r_addr, const char *devname, rf24_pa_dbm_e PA_level, rf24_datarate_e Data_rate)
{
  strcpy(_devname, devname);
  radio.begin();
  if (use_ack)
  {
    radio.enableAckPayload(); // Allow optional ack payloads
    radio.setAutoAck(1);
    radio.enableDynamicPayloads();
  }
  radio.openWritingPipe(addresses[w_addr]);
  radio.openReadingPipe(1, addresses[r_addr]);
  // radio.setPALevel(PA_level);
  // radio.setDataRate(Data_rate);
  radio.setRetries(0, 15);
  radio.startListening();
}
bool myRF24::RFwrite(const char *msg)
{
  char _msg[32];
  radio.stopListening();
  strcpy(_msg, msg);
  if (!radio.write(&_msg, sizeof(_msg)))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
bool myRF24::RFwrite(const char *msg, const int arraySize, const int len)
{
  RFmsg payload;
  byte P_iterator = 0;
  radio.stopListening();

  byte numPackets = (int)(arraySize / len);
  if (arraySize % len > 0)
  {
    numPackets++;
  }

  payload.tot_msgs = numPackets;
  payload.tot_len = arraySize;
  strcpy(payload.dev_name, _devname); /* who is sending the message */
  while (P_iterator < numPackets)
  {
    const char *ptr1 = msg + P_iterator * (len);
    strncpy(payload.payload, ptr1, len);
    payload.payload[len] = '\0';
    payload.msg_num = P_iterator;
    if (radio.write(&payload, sizeof(payload)))
    {
      P_iterator++;
    }
    else
    {
      if (debug_mode)
      {
        Serial.print("Error sending packet #");
        Serial.print(payload.msg_num);
        Serial.print("/");
        Serial.println(payload.tot_msgs - 1);
      }
      return 0;
    }
  }
  if (payload.msg_num == numPackets - 1 && payload.tot_len == strlen(msg))
  {
    if (debug_mode)
    {
      Serial.print("Sent ");
      Serial.print(arraySize);
      Serial.print(" bytes");
      Serial.print(" as ");
      Serial.print(numPackets);
      Serial.print("/");
      Serial.print(P_iterator);
      Serial.println(" packets.");
    }
    return 1;
  }
  else
  {
    return 0;
  }
}
bool myRF24::RFread(char out[], int fail_micros)
{
  radio.startListening();

  if (!_wait4Rx(fail_micros)) /* wait 200ms for incoming message */
  {
    return 0;
  }
  else
  {
    char _readmsg[32];
    radio.read(&_readmsg, sizeof(_readmsg));
    strcpy(out, _readmsg); /* now using JSON, return all message */
    return 1;
  }
}
bool myRF24::RFread(char out[], const char *key, int fail_micros)
{
  radio.startListening();
  StaticJsonDocument<80> DOC;

  if (!_wait4Rx(fail_micros)) /* wait 200ms for incoming message */
  {
    return 0;
  }
  else
  {
    char _readmsg[32];
    radio.read(&_readmsg, sizeof(_readmsg));
    deserializeJson(DOC, (const char *)_readmsg);
    if (DOC.containsKey(key))
    {
      const char *outmsg = DOC[key];
      strcpy(out, outmsg);
      return 1;
    }
    else
    {
      strcpy(out, _readmsg); // if key not present, return all message
      return 0;
    }
  }
}

bool myRF24::RFread2(char out[])
{
  radio.startListening();

  if (radio.available())
  {
    RFmsg payload;
    strcpy(out, "");
    while (_wait4Rx(50))
    {
      radio.read(&payload, sizeof(payload));
      strcat(out, payload.payload);
    }

    if (payload.tot_len == strlen(out))
    {
      return 1;
    }
    else
    {
      Serial.println("BAD LEN");
      return 0;
    }
  }
  else
  {
    return 0;
  }
}
void myRF24::genJSONmsg(char a[], const char *msg_t, const char *key, const char *value)
{
  sprintf(a, "{\"from\":\"%s\",\"msg_type\":\"%s\",\"payload0\":\"%s\",\"payload1\":\"%s\"}", _devname, msg_t, key, value);
  if (debug_mode)
  {
    Serial.println(a);
    // StaticJsonDocument<450> DOC;
    // deserializeJson(DOC, (const char *)a);
    // serializeJsonPretty(DOC, Serial);
  }
}
bool myRF24::_wait4Rx(int timeFrame)
{
  bool timeout = false;
  unsigned long started_waiting_at = micros();
  while (!radio.available()) // While nothing is received
  {
    if (micros() - started_waiting_at > timeFrame * 1000UL) // If waited longer than 200ms, indicate timeout and exit while loop
    {
      timeout = true;
      break;
    }
  }
  if (timeout)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
void myRF24::_printStruct(RFmsg &msg)
{
  Serial.print("msg_num: ");
  Serial.println(msg.msg_num);
  Serial.print("tot_msgs: ");
  Serial.println(msg.tot_msgs);
  Serial.print("tot_len: ");
  Serial.println(msg.tot_len);
  Serial.print("payload: ");
  Serial.println(msg.payload);
  Serial.print("dev_name: ");
  Serial.println(msg.dev_name);
}
