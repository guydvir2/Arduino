#include <myRF24.h>
#include <ArduinoJson.h>

// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define ROLE 1 // 0:Reciever ( ESP8266 also connected to WiFi) 1: Sender ( Pro-Micro with RF24 log range anttenna)

#if ROLE == 1 /*sender*/
const byte w_address = 1;
const byte r_address = 0;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;
const char *dev_name = "send_PRO"; /*8 letters max*/

#elif ROLE == 0 /*Receiver*/
const byte w_address = 0;
const byte r_address = 1;
const byte CE_PIN = D4;
const byte CSN_PIN = D2;
const char *dev_name = "Recv_ESP"; /*8 letters max*/
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// #if ROLE == 0

// #include <myIOT2.h>
// #define DEV_TOPIC "RF24_PORT"
// #define PREFIX_TOPIC "myHome"
// #define GROUP_TOPIC ""
// #define ADD_MQTT_FUNC addiotnalMQTT

// myIOT2 iot;

// void startIOTservices()
// {
//   iot.useSerial = true;
//   iot.useWDT = true;
//   iot.useOTA = true;
//   iot.useResetKeeper = false;
//   iot.useextTopic = false;
//   iot.resetFailNTP = true;
//   iot.useDebug = false;
//   iot.debug_level = 0;
//   iot.useNetworkReset = true;
//   iot.noNetwork_reset = 2;
//   strcpy(iot.deviceTopic, DEV_TOPIC);
//   strcpy(iot.prefixTopic, PREFIX_TOPIC);
//   strcpy(iot.addGroupTopic, GROUP_TOPIC);
//   iot.start_services(addiotnalMQTT);
// }
// void addiotnalMQTT(char *incoming_msg)
// {
//   char msg[150];
//   char msg2[20];
//   if (strcmp(incoming_msg, "status") == 0)
//   {
//     sprintf(msg, "Status: OK");
//     iot.pub_msg(msg);
//   }
//   else if (strcmp(incoming_msg, "help2") == 0)
//   {
//     sprintf(msg, "Help: Commands #3 - [NEW]");
//     iot.pub_msg(msg);
//   }
//   else if (strcmp(incoming_msg, "ver2") == 0)
//   {
//     sprintf(msg, "Ver: Ver:%s", VER);
//     iot.pub_msg(msg);
//   }
// }

// #endif

void ask_asnwer()
{
  static long lastrun = 0;

#if ROLE == 0 /*receiver*/

  radio.debug_mode = false;
  char income_msg[250];
  if (radio.RFread2(income_msg)) /* get question */
  {
    Serial.print("got question: ");
    Serial.println(income_msg);

    char temp_ans[150];
    char pload0[50];
    char pload1[20];
    char msgtype[10];
    StaticJsonDocument<300> DOC;
    deserializeJson(DOC, income_msg);

    if (strcmp(DOC["msg_type"], "q") == 0)
    { /* this is question msg */
      strcpy(msgtype, "ans");
      if (strcmp(DOC["payload0"], "clk") == 0)
      {
        // iot.get_timeStamp();
        // strcpy(pload1, iot.timeStamp);
        strcpy(pload0, "clk");
      }
      else
      {
        strcpy(pload0, "boot");
        sprintf(pload1, "%.4f [seconds]", (float)millis() / 1000);
      }
    }

    radio.genJSONmsg(temp_ans, msgtype, pload0, pload1);
    if (radio.RFwrite(temp_ans, strlen(temp_ans))) /* sending an answer */
    {
      Serial.print("Sending answer: ");
      Serial.println(temp_ans);
    }
    else
    {
      Serial.println("Error sending answer");
    }
  }

#elif ROLE == 1 /* asking a question */

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
    if (ask % 2 == 0)
    {
      radio.genJSONmsg(outmsg, "q", "clk", carray);
    }
    else
    {
      radio.genJSONmsg(outmsg, "q", "boot", carray);
    }

    // radio.genJSONmsg(outmsg, "q", "clk", carray);
    if (radio.RFwrite(outmsg, strlen(outmsg))) /*sending question*/
    {
      Serial.print("Question sent:");
      Serial.println(outmsg);
      char get_ans[200];
      strcpy(get_ans, "");
      while (!radio.RFread2(get_ans))
        ;

      Serial.print("got asnwer: ");
      Serial.println(get_ans);

      // else
      // {
      //   Serial.println("no answer received");
      // }
    }
    else
    {
      Serial.println(F("fail sending msg"));
    }
  }
#endif
}
bool send(const char *mst_t, const char *p0, const char *p1)
{
  char outmsg[250];
  radio.genJSONmsg(outmsg, mst_t, p0, p1);
  if (radio.RFwrite(outmsg, strlen(outmsg)))
  {
    Serial.print("Question sent:");
    Serial.println(outmsg);
    return 1;
  }
  else
  {
    Serial.print("failed sending:");
    Serial.println(outmsg);
    return 0;
  }
}
void setup()
{
  // #if ROLE == 0
  //   startIOTservices();
  // #else
  Serial.begin(9600);
  // #endif
  radio.startRF24(w_address, r_address, dev_name);
}

void loop()
{
  static unsigned long question_clock = 0;
  // #if ROLE == 0
  //   iot.looper();
  // #endif

  // ask_asnwer();

  // // ~~~~~~~~~ Wait for Questions ~~~~~~~~~~~~
  // char inmsg[200];
  // if (radio.RFread2(inmsg))
  // {
  //   Serial.print("IN: ");
  //   Serial.println(inmsg);

  //   // char pload1[50];
  //   // StaticJsonDocument<300> DOC;
  //   // deserializeJson(DOC, (const char *)inmsg);

  //   // if (strcmp(DOC["msg_type"], "q") == 0)
  //   // {
  //   //   if (strcmp(DOC["payload0"], "boot") == 0)
  //   //   {
  //   //     // sprintf(pload1, "%d [sec]", (int)millis()/1000);
  //   //     // Serial.println(pload1);
  //   //     send("ans", "boot", "pload1");
  //   //   }
  //   // }
  //   // else if (strcmp(DOC["msg_type"], "ans") == 0)
  //   // {
  //   //   Serial.print("got Answer: ");
  //   //   Serial.println(inmsg);
  //   // }
  //   // else if (strcmp(DOC["msg_type"], "info") == 0)
  //   // {
  //   // }
  //   // else if (strcmp(DOC["msg_type"], "alert") == 0)
  //   // {
  //   // }
  //   // else
  //   // {
  //   //   Serial.print("some error: ");
  //   //   Serial.println(inmsg);
  //   // }
  // }
  // // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // // ~~~~~~~ Sending Questions ~~~~~~~~
  // // if (ROLE == 0)
  // // {
  // //   if (millis() - question_clock > 5234)
  // //   {
  // //     char a[20];
  // //     static int q = 0;
  // //     sprintf(a, "#%d", q);
  // //     send("q", "boot", a);
  // //     question_clock = millis();
  // //     q++;
  // //   }
  // // }
  // else if (ROLE == 1)
  // {
  //   if (millis() - question_clock > 5000)
  //   {
  //     char a[20];
  //     static int q = 0;
  //     sprintf(a, "#%d", q);
  //     send("q", "boot", a);
  //     question_clock = millis();
  //     q++;
  //   }
  // }

  if (ROLE == 0)
  {
    char inmsg[200];
    radio.debug_mode = true;
    if (radio.RFread2(inmsg))
    {
      Serial.print("IN: ");
      Serial.println(inmsg);
    }
  }
  else if (ROLE == 1)
  {
    // radio.debug_mode = true;
    if (millis() - question_clock > 2000)
    {
      char outmsg[200];
      strcpy(outmsg, "123");
      if (radio.RFwrite(outmsg, strlen(outmsg)))
      {
        Serial.println("SENT");
      }
      else
      {
        Serial.println("NOT-SENT");
      }
      question_clock = millis();
    }
  }
}
