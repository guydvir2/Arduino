#include <Arduino.h>

#define ROLE 1 /* 0:Reciever 1: Sender */
#define BOTH_TX_RX false
#define DEBUG_MODE false
#define MAX_PAYLOAD_SIZE 150 //

#if defined(ARDUINO_ARCH_ESP8266)
#define isESP8266 true
#endif

#include "RF24_DEFS.h"

uint8_t Tx_fails_counter = 0;
unsigned int totTx = 0;
unsigned int totRx = 0;
char sendBuffer[3][MAX_PAYLOAD_SIZE];

void (*resetFunc)(void) = 0;

void send_timely_msgs()
{
  char qwerty[60];
  static unsigned int counter = 0;
  static unsigned long last_msg = 0;

  int randnum = random(5000, 15000);

  while (millis() - last_msg > randnum)
  {
    last_msg = millis();
    sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, Tx_fails_counter, millis() / 1000);
    bool sent_OK = radio.RFwrite(qwerty);
    if (!sent_OK)
    {
      sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, ++Tx_fails_counter);
      Serial.println(qwerty);
    }
#if DEBUG_MODE && sent_OK
    Serial.println(qwerty);
#endif
  }
}
void incomeMSG_cb(char *from, char *msg)
{
  char qwerty[MAX_PAYLOAD_SIZE];
  sprintf(qwerty, "Rx[#%d]: from[%s]; msg[%s]", ++totRx, from, msg);
  Serial.println(qwerty);
}

void setup()
{
  Serial.begin(115200);
  RF24_init();
}

void loop()
{
  // if (BOTH_TX_RX)
  // {
  //   RF24_Rx_looper();
  //   send_timely_msgs();
  // }
  // else if (ROLE == 0)
  // {
  //   RF24_Rx_looper();
  // }
  // else if (ROLE == 1)
  // {
  //   send_timely_msgs();
  // }

  // if (millis() > 30000)
  // {
  //   resetFunc();
  // }

  if (ROLE == 0)
  {
    RF24_Rx_looper();
  }
  else if (ROLE == 1)
  {
    send_timely_msgs();
  }
  radio.wellness_Watchdog();
}