#include "myRF24.h"

myRF24::myRF24(uint8_t CE_PIN, uint8_t CSN_PIN) : radio(CE_PIN, CSN_PIN)
{
}
bool myRF24::startRF24(const uint8_t &w_addr, const uint8_t &r_addr, char *devname, uint8_t PA_level, rf24_datarate_e Data_rate, uint8_t ch)
{
  _devname = devname;
  _w_addr = w_addr;
  _r_addr = r_addr;
  _PA_level = PA_level;
  _Data_rate = Data_rate;
  _ch = ch;
  return _start();
}
bool myRF24::_RFwrite_nosplit(const char *msg)
{
  char _msg[32];
  radio.stopListening();
  strcpy(_msg, msg);
  return radio.write(&_msg, sizeof(_msg));
  // if (!radio.write(&_msg, sizeof(_msg)))
  // {
  //   return 0;
  // }
  // else
  // {
  //   return 1;
  // }
}
bool myRF24::RFwrite(const char *msg)
{
  RFmsg payload;
  radio.stopListening();
  uint8_t P_iterator = 0;
  
  payload.tot_len = strlen(msg);
  payload.tot_msgs = (int)(payload.tot_len / MSG_LEN);
  
  if (payload.tot_len % MSG_LEN > 0)
  {
    payload.tot_msgs++;
  }

  // payload.tot_msgs = numPackets;
  // payload.tot_len = arraySize;
  strncpy(payload.dev_name, _devname, DEVNAME_LEN); /* who is sending the message */

  while (P_iterator < payload.tot_msgs)
  {
    const char *ptr1 = msg + P_iterator * (MSG_LEN);
    strncpy(payload.payload, ptr1, MSG_LEN);
    payload.payload[MSG_LEN] = '\0';
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
  if (payload.msg_num == payload.tot_msgs - 1 && payload.tot_len == strlen(msg))
  {
    if (debug_mode)
    {
      Serial.print("Sent ");
      Serial.print(payload.tot_len);
      Serial.print(" bytes");
      Serial.print(" as ");
      Serial.print(payload.tot_msgs);
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
bool myRF24::RFread2(char out[], int del)
{
  radio.startListening();
  if (_wait4Rx(del))
  {
    RFmsg payload;
    strcpy(out, "");
    while (radio.available())
    {
      radio.read(&payload, sizeof(payload));
      strcat(out, payload.payload);
      delay(2); // <---- Change. withouy delay, it fails.
    }

    if (payload.tot_len == strlen(out))
    {
      return 1;
    }
    else
    {
      if (debug_mode)
      {
        Serial.println(out);
        Serial.print("payload.tot_len: ");
        Serial.println(payload.tot_len);
        Serial.print("strlen(out): ");
        Serial.println(strlen(out));
        Serial.println("Error re-construct message");
      }
      return 0;
    }
  }
  else
  {
    return 0;
  }
}
void myRF24::failDetect()
{
  if (radio.failureDetected)
  {
    radio.failureDetected = 0; // Reset the detection value
    Serial.println("Rx Failure");
    if (_start())
    {
      Serial.println("Rx Restored");
    }
    else
    {
      Serial.println("Rx Fail Restored");
    }
  }
}
bool myRF24::resetRF24()
{
  // radio.powerDown();
  // delay(10);
  // radio.powerUp();
  return _start();
}
void myRF24::wellness_Watchdog()
{
  failDetect();
  if (!radio.isChipConnected())
  {
    Serial.println("NOT_CONNECTED");
    _start();
  }
}
bool myRF24::_start()
{
  bool startOK = radio.begin();
  if (use_ack)
  {
    radio.enableAckPayload(); // Allow optional ack payloads
    radio.setAutoAck(1);
    radio.enableDynamicPayloads();
  }
  radio.openWritingPipe(addresses[_w_addr]);
  radio.openReadingPipe(1, addresses[_r_addr]);
  radio.setPALevel(_PA_level);
  radio.setDataRate(_Data_rate);
  radio.setRetries(0, 15);
  radio.setChannel(_ch);
  radio.startListening();
  return startOK;
}
bool myRF24::_wait4Rx(int timeFrame)
{
  bool timeout = false;
  unsigned long started_waiting_at = micros();

  while (!radio.available() && !timeout)
  {
    if (micros() - started_waiting_at > timeFrame * 1000UL)
    {
      timeout = true;
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