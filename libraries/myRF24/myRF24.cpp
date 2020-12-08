#include "myRF24.h"

myRF24::myRF24(int CE_PIN, int CSN_PIN) : radio(CE_PIN, CSN_PIN)
{
}
void myRF24::startRF24(const byte &w_addr, const byte &r_addr, const char *devname)
{
  strcpy(_devname, devname);
  radio.begin();
  // radio.enableAckPayload(); // Allow optional ack payloads
  // radio.setAutoAck(1);
  // radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[w_addr]);
  radio.openReadingPipe(1, addresses[r_addr]);
  radio.setPALevel(RF24_PA_MIN);
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
bool myRF24::RFwrite(const char *msg, const char *key)
{
  char _msg[32];
  radio.stopListening();
  StaticJsonDocument<80> DOC;

  strcpy(_msg, msg);
  DOC[key] = msg;
  DOC["id"] = _devname;
  serializeJson(DOC, _msg);
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
  payload.tot_len = arraySize - 1;
  strcpy(payload.dev_name, _devname); /* who is sending the message */

  while (P_iterator < numPackets)
  {
    const char *ptr1 = msg + P_iterator * (len);
    strncpy(payload.payload, ptr1, len);
    payload.payload[len] = '\0';
    payload.msg_num = P_iterator;
    radio.write(&payload, sizeof(payload));
    P_iterator++;
  }
  Serial.print("Sent ");
  Serial.print(arraySize);
  Serial.print(" bytes");
  Serial.print(" as ");
  Serial.print(numPackets);
  Serial.print("/");
  Serial.print(P_iterator);
  Serial.println(" packets.");
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
  const int MAX_PACKETS = 20;
  const int MAX_PACKET_LEN = 25;
  char packets[MAX_PACKETS][MAX_PACKET_LEN];

  if (radio.available())
  {
    RFmsg payload;
    radio.read(&payload, sizeof(payload));
    strcpy(packets[payload.msg_num], payload.payload);

    if (payload.msg_num == payload.tot_msgs - 1) /* reaching last message */
    {
      int recv_msg_len = 0;
      byte recv_packets = 0;
      strcpy(out, "");
      for (int i = 0; i < payload.tot_msgs; i++)
      {
        recv_packets++;
        recv_msg_len += strlen(packets[i]);
        strcat(out, packets[i]);
      }

      if (payload.tot_len == recv_msg_len && recv_packets == payload.tot_msgs)
      {
        // Serial.println("Message received OK");
        return 1;
      }
      else
      {
        Serial.println("Message failed receiving");
        return 0;
      }
    }
    else{
      return 0;
    }
  }
}
void myRF24::RFans()
{
  char outmsg[32];
  if (RFread(outmsg, "Q"))
  {
    Serial.print("got msg: ");
    Serial.println(outmsg);
    if (strcmp("Q", "clk"))
    {
      if (RFwrite("1234", "clk"))
      {
        Serial.println("reply sent");
      }
    }
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