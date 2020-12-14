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

  if (debug_mode)
  {
    Serial.begin(9600);
  }
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
  Serial.println("write");
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
  // _printStruct(payload);

  // Serial.print("numPackets: ");
  // Serial.println(numPackets);
  while (P_iterator < numPackets)
  {
    const char *ptr1 = msg + P_iterator * (len);
    strncpy(payload.payload, ptr1, len);
    payload.payload[len] = '\0';
    payload.msg_num = P_iterator;

    Serial.print("iteration: ");
    Serial.println(P_iterator);
    if (radio.write(&payload, sizeof(payload)))
    {
      P_iterator++;
    }
    else
    {
      Serial.println("Error");
      return 0;
    }
  }
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
    if (_wait4Rx()) //radio.available())
    {
      RFmsg payload;
  //     radio.read(&payload, sizeof(payload));
  //     if (payload.msg_num == 0) /* first msg in */
  //     {
  //       /* check to see not exceeding max length */
  //       if (payload.tot_len > MAX_PACKET_LEN * MAX_PACKETS - 1)
  //       {
  //         if (debug_mode)
  //         {
  //           char t[100];
  //           sprintf(t, "Error. %d bytes. allowed %d bytes", payload.tot_len, MAX_PACKET_LEN * MAX_PACKETS - 1);
  //           Serial.println(t);
  //         }
  //         return 0;
  //       } /* End */

  //       strcpy(packets[payload.msg_num], payload.payload);
  //     }
  //     else /* next msgs */
  //     {
  //       while (_wait4Rx() && payload.msg_num < payload.tot_msgs - 1)
  //       {
  //         radio.read(&payload, sizeof(payload));
  //         strcpy(packets[payload.msg_num], payload.payload);
  //         _printStruct(payload);
  //         Serial.println("D");
  //       }
  //       if (payload.msg_num == payload.tot_msgs - 1) /* reaching last message */
  //       {
  //         int recv_msg_len = 0;
  //         byte recv_packets = 0;
  //         strcpy(out, "");
  //         for (int i = 0; i < payload.tot_msgs; i++)
  //         {
  //           recv_packets++;
  //           recv_msg_len += strlen(packets[i]);
  //           strcat(out, packets[i]);
  //           if (debug_mode)
  //           {
  //             Serial.println(packets[i]);
  //           }
  //         }
  //         return 1;
  //       }
  //       else
  //       {
  //         Serial.println("BAD PACKETS");
  //         return 0;
  //       }
  //     }
  //   }
  //   else
  //   {
  //     Serial.println("BBBBAAAA");
  //     return 0;
  //   }
  // }
}
void myRF24::genJSONmsg(char a[], const char *msg_t, const char *key, const char *value)
{
  sprintf(a, "{\"sender\":\"%s\",\"msg_type\":\"%s\",\"payload0\":\"%s\",\"payload1\":\"%s\"}", _devname, msg_t, key, value);
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
// bool myRF24::_verifyRx(char out[], char packets[][])
// {
//   // if (payload.msg_num == payload.tot_msgs - 1) /* reaching last message */
//   // {
//   //   int recv_msg_len = 0;
//   //   byte recv_packets = 0;
//   //   strcpy(out, "");
//   //   for (int i = 0; i < payload.tot_msgs; i++)
//   //   {
//   //     recv_packets++;
//   //     recv_msg_len += strlen(packets[i]);
//   //     strcat(out, packets[i]);
//   //     if (debug_mode)
//   //     {
//   //       Serial.println(packets[i]);
//   //     }
//   //   }
//   // }

//   // if (payload.tot_len == recv_msg_len && recv_packets == payload.tot_msgs)
//   // {
//   //   if (debug_mode)
//   //   {
//   //     Serial.println(payload.tot_len);
//   //     Serial.print("recv_msg_len: ");
//   //     Serial.println("Message received OK");
//   //     Serial.println(out);
//   //   }
//   //   return 1;
//   // }
//   // else
//   // {
//   //   if (debug_mode)
//   //   {
//   //     Serial.println("Message failed receiving");
//   //     Serial.print("payload.tot_len: ");
//   //     Serial.println(payload.tot_len);
//   //     Serial.print("recv_msg_len: ");
//   //     Serial.println(recv_msg_len);
//   //     Serial.print("recv_packets: ");
//   //     Serial.println(recv_packets);
//   //     Serial.print("payload.tot_msgs: ");
//   //     Serial.println(payload.tot_msgs);
//   //     Serial.println(out);
//   //   }
//   //   return 0;
//   // }
// }