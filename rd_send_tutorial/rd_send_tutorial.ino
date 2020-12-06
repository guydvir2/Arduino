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
  // radio.enableAckPayload();
  // radio.enableDynamicPayloads();
  radio.openWritingPipe(addresses[w_addr]);    // 00001
  radio.openReadingPipe(1, addresses[r_addr]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  // radio.writeAckPayload(0, &Ack_msg, sizeof(Ack_msg));
  // radio.setRetries(15, 15);
  radio.startListening();
}

void burstMSG()
{
  static int send_id = 0;
  Serial.println("\n±±±±±±±");
  char msgs[][15] = {"1234567890GUYD\0", "1234567890DVIR\0", "1234567890ANNA\0", "1234567890OZZZ\0", "1234567890SHAX\0"};
  // char msgs[][15] = {"GUYD\0", "DVIR\0", "ANNA\0", "ORZZ\0", "Shac\0"};


  for (int i = 0; i < 5; i++)
  {
    char temp[30];
    char add[] = "AAACCCCBBBBEEEEEFFF_";
    // sprintf(temp, "%s_%sid_#%d", msgs[i], add, send_id);

    if (radio.write(&msgs[i], sizeof(msgs[i])))
    {
      Serial.println(i);
    //   radio.startListening();
    //   delay(5);
    //   if (radio.available())
    //   {
    //     // byte rec;
    //     // radio.read(&rec, 1);
    //     char ack[32];
    //     radio.read(&ack, sizeof(ack));
    //     Serial.println(ack);
    //   }
    //   radio.stopListening();
    }
    else
    {
      Serial.print("fail sending msg:");
      Serial.println(i);
    }
  }
  send_id++;
  Serial.println("~~~~~~~~~END~~~~~~~");

  // while (P_iterator < numPackets)
  // {

  //   if (radio.write(&payload, sizeof(payload)))
  //   {
  //     char a[50];
  //     sprintf(a, "msg #%d sent OK: %s", P_iterator, payload.payload);
  //     Serial.println(a);

  //   }
  //   else
  //   {
  //     char a[50];
  //     sprintf(a, "msg #%d failed: %s", P_iterator, payload.payload);
  //   }
  //   P_iterator++;
  // }
}

void setup()
{
  Serial.begin(9600);
  startRF24(w_address, r_address);
}

void loop()
{

  radio.stopListening();

  // char send_msg[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  burstMSG();
  delay(5000);
  // Serial.println("±±±±±±±±");
}
