#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <ArduinoJson.h>
#include "myIOT_settings.h"

// ~~~~~~~ RF24 Functions ~~~~~~~~~~~~~~~~~
const byte addresses[][6] = {"00001", "00002"};
const byte w_address = 0;
const byte r_address = 1;
const byte CE_PIN = D4;
const byte CSN_PIN = D2;
const char *RF_NAME = "Port";
char Ack_msg[32];

RF24 radio(CE_PIN, CSN_PIN);
struct RFmsg
{
  byte msg_num;
  byte tot_msgs;
  char payload[30];
};
RFmsg payload;

void startRF24(const byte &w_addr, const byte &r_addr)
{
  radio.begin();
  // radio.enableAckPayload(); // Allow optional ack payloads
  radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[w_addr]);    // 00001
  radio.openReadingPipe(1, addresses[r_addr]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  strcpy(Ack_msg, "SHIIIT");
  // radio.writeAckPayload(1, &Ack_msg, sizeof(Ack_msg));
  radio.startListening();
}

void loopNRF24()
{
  radio.startListening();
  delay(5);

  if (radio.available())
  {

    // uint8_t a = radio.getDynamicPayloadSize();
    // Serial.println(a);

    // StaticJsonDocument<80> DOC;
    char text[35] = {0};
    radio.read(&text, sizeof(text));
    Serial.println(text);
    const char msg[] = "gotThat";
    radio.writeAckPayload(1, &msg, sizeof(msg));
    // deserializeJson(DOC, text);
    // // delay(100);
    // const char *msg = DOC["msg"];
    // Serial.print("msg is: ");
    // Serial.println(msg);
    //   // Serial.println((strcmp(msg, "clk") == 0));
    // serializeJsonPretty(DOC, Serial);

    //   // Serial.println(text);
    // delay(5);
    // radio.stopListening();
    // if (strcmp(msg, "clk") == 0)
    // {
    // iot.get_timeStamp();
    // Serial.println("GOT_IT");
    // if(strcmp(msg,"")==0){
    //   Serial.println("Shit it is empty ");
    // }
    // sendRFmsg("timeStamp");
    // }
  }
  // sendRFmsg("timeStamp");
}
// void ack_loop()
// {
//   byte pipeNo;
//   // radio.startListening();
//   while (radio.available(&pipeNo)) //&pipeNo
//   {
//     char outmsg[32];
//     char t[32] = "take2";

//     readRFmsg("msg", outmsg);
//     if (strcmp(outmsg, "clk"))
//     {
//       Serial.println("Yes- it is clk");
//       // iot.get_timeStamp();
//       // radio.writeAckPayload(1, &iot.timeStamp, sizeof(iot.timeStamp));
//       sendRFmsg("GOOOOD");
//     }

//     // iot.get_timeStamp();
//     // radio.stopListening();
//     // sendRFmsg("OK");
//     // delay(10);
//     // radio.startListening();
//     // radio.writeAckPayload(1, &t, sizeof(t));
//   }
//   // iot.looper();
//   delay(100);
// }

bool RFwrite(const char *msg, const char *key = "msg")
{
  StaticJsonDocument<80> DOC;
  radio.stopListening(); // First, stop listening so we can talk.
  char _msg[32];
  strcpy(_msg, msg);
  DOC["id"] = RF_NAME;
  DOC[key] = msg;
  serializeJson(DOC, _msg);
  if (!radio.write(&_msg, sizeof(_msg)))
  {
    return 0;
    Serial.println(F("failed"));
  }
  else
  {
    return 1;
  }
}
bool RFread(char out[] = nullptr, const char *key = nullptr, unsigned long fail_micros = 200000)
{
  bool timeout = false;                        // Set up a variable to indicate if a response was received or not
  unsigned long started_waiting_at = micros(); // Set up a timeout period, get the current microseconds

  radio.startListening(); // Now, continue listening

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
    char inmsg[32];
    radio.read(&inmsg, sizeof(inmsg));
    Serial.println(inmsg);
    // if (key != nullptr)
    // {
    //   StaticJsonDocument<80> DOC;
    //   deserializeJson(DOC, inmsg);
    //   if (DOC.containsKey(key))
    //   {
    //     const char *outmsg = DOC[key];
    //     if (out != nullptr)
    //     {
    //       strcpy(out, outmsg);
    //       return true;
    //     }
    //   }
    //   else
    //   {
    //     strcpy(out, inmsg); // if key not present, return all message
    //     return 0;
    //   }
    // }
    // else
    // {
    //   strcpy(out, inmsg); // if key not present, return all message
    //   return 0;
    // }

    return 0;
  }
}
void RFans()
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

void setup()
{
  // startIOTservices();
  Serial.begin(9600);
  startRF24(w_address, r_address);
}

void loop()
{
  if (radio.available())
  {
    char a[32];
    radio.read(&a, sizeof(a));
    Serial.println()
    // if (payload.msg_num == 0)
    // {
    //   Serial.print("total packets: ");
    //   Serial.println(payload.tot_msgs);
    //   while (payload.msg_num < payload.tot_msgs )
    //   {
    //     Serial.print("msg #");
    //     Serial.print(payload.msg_num);
    //     Serial.print("/");
    //     Serial.print(payload.tot_msgs);
    //     Serial.print(": ");
    //     Serial.println(payload.payload);
    //     radio.read(&payload, sizeof(payload));
    //   }
    // }
    // else{
    //   Serial.println("pre-mature");
    // }
  }
  delay(5);
  // Serial.println("loop");
}
