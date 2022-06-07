#include "myRF24.h"

myRF24::myRF24(uint8_t CE_PIN, uint8_t CSN_PIN) : radio(CE_PIN, CSN_PIN)
{
}
bool myRF24::startRF24(const uint8_t &w_addr, const uint8_t &r_addr, char *devname, uint8_t PA_level, rf24_datarate_e Data_rate, uint8_t ch)
{
  _ch = ch;
  _w_addr = w_addr;
  _r_addr = r_addr;
  _devname = devname;
  _PA_level = PA_level;
  _Data_rate = Data_rate;

  return _start();
}
bool myRF24::RFwrite(const char *msg)
{
  RFmsg _payload;
  radio.stopListening();
  _erase_struct(_payload);

  if (debug_mode)
  {
    char msg2[200];
    sprintf(msg2, "out_msg: from[%s] msg[%s]", _devname, msg);
    Serial.println(msg2);
  }

  _payload.tot_len = strlen(msg);
  _payload.tot_msgs = _payload.tot_len / MSG_LEN;
  if (_payload.tot_len % MSG_LEN > 0)
  {
    _payload.tot_msgs++;
  }
  strncpy(_payload.dev_name, _devname, DEVNAME_LEN);

  for (uint8_t i = 0; i < _payload.tot_msgs; i++)
  {
    strcpy(_payload.payload, "");
    for (uint8_t n = 0; n < MSG_LEN; n++)
    {
      _payload.payload[n] = (char)msg[n + i * MSG_LEN];
    }
    _payload.payload[MSG_LEN] = '\0';
    if (radio.write(&_payload, sizeof(_payload)))
    {
      _payload.msg_num++;
    }
    else
    {
      if (debug_mode)
      {
        Serial.print("[SENT_FAIL]. ");
        Serial.print(" packet [#");
        Serial.print(_payload.msg_num);
        Serial.println("]");
      }
      return 0;
    }
  }
  if (_payload.msg_num == _payload.tot_msgs && _payload.tot_len == strlen(msg))
  {
    if (debug_mode)
    {
      Serial.print("[SENT_OK] [");
      Serial.print(_payload.tot_len);
      Serial.print(" bytes] [");
      Serial.print(_payload.tot_msgs);
      Serial.println(" packets]");
    }
    return 1;
  }
  else
  {
    return 0;
  }
}

bool myRF24::RFread(char out[], char from[], int del)
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
    strcpy(from, payload.dev_name);

    if (debug_mode)
    {
      char msg[200];
      sprintf(msg, "in_msg: from[%s] msg[%s]", from, out);
      Serial.println(msg);
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
bool myRF24::failDetect()
{
  if (radio.failureDetected)
  {
    radio.failureDetected = 0; // Reset the detection value
    Serial.println("Rx Failure");
    if (_start())
    {
      Serial.println("Rx Restored");
      return 0;
    }
    else
    {
      Serial.println("Rx Fail Restored");
      return 1;
    }
  }
  else
  {
    return 0;
  }
}
bool myRF24::resetRF24()
{
  radio.powerDown();
  delay(10);
  radio.powerUp();
  return _start();
}
void myRF24::wellness_Watchdog()
{
  if (!radio.isChipConnected()&&failDetect())
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
void myRF24::_erase_struct(RFmsg &_payload)
{
  _payload.tot_len = 0;
  _payload.tot_msgs = 0;
  _payload.msg_num = 0;
  strcpy(_payload.payload, "");
  strcpy(_payload.dev_name, "");
}