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
bool myRF24::RFwrite(const char *msg, const char *key)
{
  char _msg[32];
  radio.stopListening();
  strcpy(_msg, msg);
  if (key != nullptr)
  {
    StaticJsonDocument<80> DOC;
    DOC["id"] = _devname;
    DOC[key] = msg;
    serializeJson(DOC, _msg);
  }
  if (!radio.write(&_msg, sizeof(_msg)))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
bool myRF24::RFread(char out[], const char *key, unsigned long fail_micros)
{
  radio.startListening();

  bool timeout = false;                        
  unsigned long started_waiting_at = micros();

  while (!radio.available()) // While nothing is received
  {
    if (micros() - started_waiting_at > fail_micros) // If waited longer than 200ms, indicate timeout and exit while loop
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
    char _readmsg[32];
    radio.read(&_readmsg, sizeof(_readmsg));

    if (key != nullptr)
    {
      StaticJsonDocument<80> DOC;
      deserializeJson(DOC, (const char *)_readmsg);
      if (DOC.containsKey(key))
      {
        const char *outmsg = DOC[key];
        if (out != nullptr)
        {
          strcpy(out, outmsg);
          return true;
        }
      }
      else
      {
        strcpy(out, _readmsg); // if key not present, return all message
        return 0;
      }
    }
    else
    {
      strcpy(out, _readmsg); // if key not present, return all message
      return 1;
    }
  }
}
void myRF24::splitMSG(const char *msg, const int arraySize, const int len)
{
  RFmsg payload;
  byte P_iterator = 0;
  byte numPackets = (int)(arraySize / len);
  payload.tot_len = arraySize - 1;
  strcpy(payload.dev_name, _devname);

  radio.stopListening();
  if (arraySize % len > 0)
  {
    numPackets++;
  }
  payload.tot_msgs = numPackets;
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
  delay(5000);
}
bool myRF24::readsplit(char recvMessage[])
{
  radio.startListening();
  char packets[20][25];
  if (radio.available())
  {
    RFmsg payload;
    radio.read(&payload, sizeof(payload));
    strcpy(packets[payload.msg_num], payload.payload);

    if (payload.msg_num == payload.tot_msgs - 1)
    {
      int recv_msg_len = 0;
      byte recv_packets = 0;

      for (int i = 0; i < payload.tot_msgs; i++)
      {
        // Serial.print(packets[i]);
        recv_msg_len += strlen(packets[i]);
        recv_packets++;
        if (i == 0)
        {
          strcpy(recvMessage, packets[i]);
        }
        else
        {
          strcat(recvMessage, packets[i]);
        }
      }

      if (payload.tot_len == recv_msg_len && recv_packets == payload.tot_msgs)
      {
        Serial.println("Message received OK");
        return 1;
      }
      else
      {
        Serial.println("Message failed receiving");
        return 0;
      }
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
bool myRF24::_wait4Rx(int timeFrame){
  bool timeout = false;
  unsigned long started_waiting_at = micros();
  while (!radio.available()) // While nothing is received
  {
    if (micros() - started_waiting_at > timeFrame*1000UL) // If waited longer than 200ms, indicate timeout and exit while loop
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