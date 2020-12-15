#include <myRF24.h>
#include <ArduinoJson.h>

// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define ROLE 0 // 0:Reciever 1: Sender
#if ROLE == 1  // sender
const byte w_address = 1;
const byte r_address = 0;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;
const char *dev_name = "send_PRO"; /* 8 letters max*/
#elif ROLE == 0                    /* Receiver*/
const byte w_address = 0;
const byte r_address = 1;
const byte CE_PIN = D4;
const byte CSN_PIN = D2;
const char *dev_name = "Recv_ESP";
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if ROLE == 0
#define DEV_TOPIC "RF24_PORT"
#define PREFIX_TOPIC "myHome"
#define GROUP_TOPIC ""
#include <myIOT2.h>
myIOT2 iot;

#define ADD_MQTT_FUNC addiotnalMQTT
void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = false;
  iot.useextTopic = false;
  iot.resetFailNTP = true;
  iot.useDebug = false;
  iot.debug_level = 0;
  iot.useNetworkReset = true;
  iot.noNetwork_reset = 2;
  strcpy(iot.deviceTopic, DEV_TOPIC);
  strcpy(iot.prefixTopic, PREFIX_TOPIC);
  strcpy(iot.addGroupTopic, GROUP_TOPIC);
  iot.start_services(addiotnalMQTT);
}

void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: OK");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help: Commands #3 - [NEW]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "Ver: Ver:%s", VER);
    iot.pub_msg(msg);
  }
}

#endif

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
      char ans2[100];

      sprintf(ans2, "%.4f [seconds]", (float)millis() / 1000);
      radio.genJSONmsg(temp_ans, "a", "recv", ans2);
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
  }
}
void setup()
{
#if ROLE == 0
  startIOTservices();
#else
  Serial.begin(9600);
#endif
  radio.startRF24(w_address, r_address, dev_name);
}

void loop()
{
#if ROLE == 0
  iot.looper();
#endif

  ask_asnwer();
}
