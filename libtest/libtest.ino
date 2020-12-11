#include <myRF24.h>
#include <ArduinoJson.h>

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

// #if ROLE==0
// #include <myIOT2.h>
// myIOT2 iot;
// #endif

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
// bool radioNumber = ROLE;
// byte addresses[][6] = {"1Node", "2Node"};
// RF24 radio(CE_PIN, CSN_PIN);


/**********************************************************/
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
      StaticJsonDocument<300> DOC;
      deserializeJson(DOC, (const char *)inmsg);
      // Serial.println(inmsg);
    }
  }
  else if (ROLE == 1)
  {
    while (millis() - lastrun > 5000)
    {
      // char send_long_msg[] = "ZXCVBNM<>ASDFGHHJKL:|QWERTYUIOP{}THIS_IS_A_VERY_LONG_MESSAGE_1234567890ABCDEFGHIJKLMNOP_BLABLABLA_KKK!@#$%^&*()";
      char send_long_msg[] = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\":false,\"useFailNTP\":true,\"useDebugLog\":true,\"useNetworkReset\":true}";

      radio.RFwrite(send_long_msg, sizeof(send_long_msg));
      lastrun = millis();
    }
  }
}
void ask_asnwer()
{
  static long lastrun = 0;

  if (ROLE == 0) /*receiver*/
  {
    radio.debug_mode = false;
    char income_msg[250];
    if (radio.RFread2(income_msg)) /* get question */
    {
      Serial.println("ֿ\n§§§§§§§§§§§§ Start ±±±±±±±±±±±±");
      Serial.print("got question: ");
      Serial.println(income_msg);

      char temp_ans[250];
      sprintf(temp_ans, "recv_time: %.2f", (float)millis() / 1000);
      // radio.genJSONmsg(temp_ans, "a", "clk", "?");
      // delay(150);
      Serial.println("sending ans:");
      if (radio.RFwrite(temp_ans, strlen(temp_ans))) /* sending an answer */
      {
        Serial.print("Sending answer: ");
        Serial.println(temp_ans);
      }
      else
      {
        Serial.println("Error sending");
      }
    }
  }
  else if (ROLE == 1) /* asking a question */
  {
    while (millis() - lastrun > 5000)
    {
      Serial.println("ֿ\n§§§§§§§§§§§§ Start Sending ±±±±±±±±±±±±");
      char outmsg[250];
      radio.debug_mode = false;
      lastrun = millis();
      static int ask = 0;

      char carray[15];
      sprintf(carray, "#%d", ask);
      ask++;

      radio.genJSONmsg(outmsg, "q", "clk", carray);
      Serial.print("Sending question:");
      Serial.println(outmsg);
      if (radio.RFwrite(outmsg, strlen(outmsg))) /*sending question*/
      {
        char get_ans[200];
        strcpy(get_ans, "");
        // delay(100);
        Serial.println("waiting for ans:");
        if (radio.RFread2(get_ans))
        {
          Serial.print("got asnwer: ");
          Serial.println(get_ans);
        }
        else
        {
          Serial.println("no answer");
        }
      }
    }
  }
}
void setup()
{
  Serial.begin(9600);
  // radio.openWritingPipe(addresses[w_address]);
  // radio.openReadingPipe(1, addresses[r_address]);
  radio.startRF24(w_address, r_address, dev_name);
}

void loop()
{
  // simple_send_recv();
  // splitmsgs_send_recv();
  ask_asnwer();
}
