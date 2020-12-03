#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoJson.h>

// ~~~~~~~ RF24 Functions ~~~~~~~~~~~~~~~~~
const byte addresses[][6] = {"00001", "00002"};
const byte w_address = 1;
const byte r_address = 0;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;
const char *RF_NAME = "sens1";
char Ack_msg[32];

RF24 radio(CE_PIN, CSN_PIN);

void startRF24(const byte &w_addr, const byte &r_addr)
{
  radio.begin();
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[w_addr]);    // 00001
  radio.openReadingPipe(1, addresses[r_addr]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  radio.writeAckPayload(0, &Ack_msg, sizeof(Ack_msg));
  radio.startListening();
}

// void ack_loop()
// {
//   static unsigned long last_transmit = 0;
//   const int send_intervals = 1000;

//   if (millis() > last_transmit + send_intervals)
//   {
//     if (sendRFmsg("clk"))
//     {
//       last_transmit = millis();
//       Serial.println("sent: OK");
//       Serial.print("acknowlodge: ");
//       while (!radio.available())
//       {
//         Serial.println("none");
//       }
//       if (radio.available())
//       {
//         char ackmsg[32];
//         radio.read(&ackmsg, sizeof(ackmsg));
//         Serial.println(ackmsg);
//       }
//       radio.stopListening();
//     }
//     delay(1000);
//   }
// }
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool RFwrite(const char *msg, const char *key = "msg")
{
  char _msg[32];
  StaticJsonDocument<80> DOC;
  radio.stopListening(); // First, stop listening so we can talk.
  strcpy(_msg, msg);
  DOC["id"] = RF_NAME;
  DOC[key] = msg;
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
bool RFread(char out[] = nullptr, const char *key = nullptr, unsigned long fail_micros = 200000)
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
      return 0;
    }
  }
}
bool RFask(const char *q, char ans[]){
  if (RFwrite(q,"Q"))
  {
    if (RFread(ans, q))
    {
      Serial.print("question: ");
      Serial.println(q);
      Serial.print("answer: ");
      Serial.println(ans);
      return 1;
    }
    else
    {
      Serial.print("Error. no such key.");
      Serial.println(ans);
      strcpy(ans, "");
      return 0;
    }
  }
  else
  {
    Serial.println("message failed to send");
    return 0;
  }
}
bool RFshare(const char *key, char value[], int ret =3, int delay_micros=1000){
  int retries = 0;
  while (retries<ret){
    if(RFwrite(key,value)){
      break;
    }
    else{
      retries++;
    }
  }
}
 void setup()
{
  Serial.begin(9600);
  startRF24(w_address, r_address);
}

void loop()
{
  // ~~~~~~~~~~ Sending Message ~~~~~~~~~~~~~
  // if (RFwrite("clk"))
  // {
  //   Serial.println("message sent OK");
  //   char t[32];
  //   if (RFread(t, "msg"))
  //   {
  //     Serial.print("got reply msg: ");
  //     Serial.println(t);
  //   }
  //   else
  //   {
  //     Serial.print("Error. no such key. got reply: ");
  //     Serial.println(t);
  //     Serial.flush();
  //   }
  // }
  // else
  // {
  //   Serial.println("message failed to send");
  // }
  // ~~~~~~~~~~~ Wait to response ~~~~~~~~~~~~~
  char q[] = "clk";
  char answer[32];
  RFask(q, answer);
  delay(5000);
}
