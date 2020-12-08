#include <myRF24.h>

// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define ROLE 1 // 0:Reciever 1: Sender
#if ROLE == 1  // sender
const byte w_address = 1;
const byte r_address = 0;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;
const char *dev_name = "send";
#elif ROLE == 0 /* Receiver*/
const byte w_address = 0;
const byte r_address = 1;
const byte CE_PIN = D4;
const byte CSN_PIN = D2;
const char *dev_name = "Recv";
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void simple_send_recv()
{
  if (ROLE == 0)
  {
    char recmsg[32];
    if (radio.RFread(recmsg))
    {
      Serial.print("msg in: ");
      Serial.println(recmsg);
    }
    else
    {
      Serial.println(recmsg);
      Serial.println("Error reading msg");
    }
  }
  else if (ROLE == 1)
  {
    if (!radio.RFwrite("TEST"))
    {
      Serial.println("Error sending");
    }
  }
}
void splitmsgs_send_recv()
{
  static long lastrun = 0;
  if (ROLE == 0)
  {
    char inmsg[250];
    if (radio.RFread2(inmsg))
    {
      Serial.println(inmsg);
    }
  }
  else if (ROLE == 1)
  {
    while (millis() - lastrun > 10000)
    {
      char send_long_msg[] = "ZXCVBNM<>ASDFGHHJKL:|QWERTYUIOP{}THIS_IS_A_VERY_LONG_MESSAGE_1234567890ABCDEFGHIJKLMNOP_BLABLABLA_KKK!@#$%^&*()";
      radio.RFwrite(send_long_msg, sizeof(send_long_msg));
      lastrun = millis();
    }
  }
}

void setup()
{
  Serial.begin(9600);
  radio.startRF24(w_address, r_address, dev_name);
}

void loop()
{
  // simple_send_recv();
  splitmsgs_send_recv();
}
