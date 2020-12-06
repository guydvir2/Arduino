#include "myRF24.h"

myRF24::myRF24(int CE_PIN, int CSN_PIN) : radio(CE_PIN, CSN_PIN)
{
}
void myRF24::startRF24(const byte &w_addr, const byte &r_addr, const char *devname)
{
  radio.begin();
  // radio.enableAckPayload(); // Allow optional ack payloads
  // radio.setAutoAck(1);
  // radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[w_addr]);    // 00001
  radio.openReadingPipe(1, addresses[r_addr]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(0, 15);
  strcpy(_devname, devname);
  radio.startListening();
}
bool myRF24::RFwrite(const char *msg, const char *key)
{
  char _msg[32];
  radio.stopListening(); // First, stop listening so we can talk.
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
  bool timeout = false;                        // Set up a variable to indicate if a response was received or not
  unsigned long started_waiting_at = micros(); // Set up a timeout period, get the current microseconds

  radio.startListening();    // Now, continue listening
  while (!radio.available()) // While nothing is received
  {
    if (micros() - started_waiting_at > fail_micros) // If waited longer than 200ms, indicate timeout and exit while loop
    {
      timeout = true;
      break;
    }
  }
  if (timeout) // Describe the results
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
  delay(200);
}
bool myRF24::readsplit()
{
  radio.startListening();
  char combined[200];
  // byte pipeNo;
  if (radio.available())
  {
    RFmsg payload;
    bool timeout = false;
    unsigned long started_waiting_at = micros();
    // while (!radio.available()) // While nothing is received
    // {
    //   if (micros() - started_waiting_at > 200000) // If waited longer than 200ms, indicate timeout and exit while loop
    //   {
    //     timeout = true;
    //     break;
    //   }
    //   else
    //   {
    //     return 0;
    //   }
    // }
    radio.read(&payload, sizeof(payload));
    if (payload.msg_num == 0)
    {
      Serial.println("Start");
    }
    else
    {
      Serial.println(payload.msg_num);
    }

  }
  return 1;
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