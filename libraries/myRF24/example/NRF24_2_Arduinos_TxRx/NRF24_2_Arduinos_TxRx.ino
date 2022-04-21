#define ROLE 0 /* 0:Reciever 1: Sender */
#define BOTH_TX_RX false

#include <myRF24.h>
#include "RF24_DEFS.h"

void convert_epoch2clock(char time_str[])
{
  uint8_t days = 0;
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;

  const uint8_t sec2minutes = 60;
  const int sec2hours = (sec2minutes * 60);
  const int sec2days = (sec2hours * 24);

  long time_delta = millis() / 1000;

  days = (int)(time_delta / sec2days);
  hours = (int)((time_delta - days * sec2days) / sec2hours);
  minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
  seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

  sprintf(time_str, "%01dd %02d:%02d:%02d", days, hours, minutes, seconds);
}
void send_timely_msgs()
{
  char qwerty[60];
  static long last_msg = 0;
  static unsigned int counter = 0;

  while (millis() - last_msg > random(2500, 7500))
  {
    char a[30];
    last_msg = millis();
    // convert_epoch2clock(a);
    sprintf(qwerty, "Tx #%d", ++counter);

    // sprintf(qwerty, "Tx #%d; upTime:%s", counter++, a);
    radio.RFwrite(qwerty);
  }
}
void incomeMSG_cb(char *from, char *msg)
{
  Serial.print("INmsg>> From[");
  Serial.print(from);
  Serial.print("] MSG[");
  Serial.print(msg);
  Serial.println("]");
}

void setup()
{
  setup_sender();
  setup_reciever();
}

void loop()
{
  if (BOTH_TX_RX)
  {
    RF24_Rx_looper();
    send_timely_msgs();
  }
  else if (ROLE == 0)
  {
    RF24_Rx_looper();
  }
  else if (ROLE == 1)
  {
    send_timely_msgs();
  }

  // radio.failDetect();
  // radio.wellness_Watchdog();
}