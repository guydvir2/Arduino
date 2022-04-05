#include <myRF24.h>

#define ROLE 0 /* 0:Reciever 1: Sender */
#define BOTH_TX_RX false

// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define DEBUG_MODE true;
#define USE_ACK false;
#if ROLE == 1
const uint8_t w_address = 1;
const uint8_t r_address = 0;
const uint8_t CE_PIN = 9;
const uint8_t CSN_PIN = 10;

char *dev_name = "Rdu_1";
const int time_resend = 2500;

#elif ROLE == 0
const uint8_t w_address = 0;
const uint8_t r_address = 1;
const uint8_t CE_PIN = 9;
const uint8_t CSN_PIN = 10;

const char *dev_name = "Rdu_0";
const int time_resend = 23456;
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void send_timely_msgs()
{
  char qwerty[60];
  static long last_msg = 0;
  static unsigned int counter = 0;

  while (millis() - last_msg > random(2500, 7500))
  {
    last_msg = millis();
    sprintf(qwerty, "msg #%d", counter++);
    radio.RFwrite(qwerty);
  }
}
void recv_msg()
{
  char from[10];
  char msg[200];
  char inmsg_buff[150];

  const uint8_t delay_read = 100;
  if (radio.RFread2(inmsg_buff, from, delay_read))
  {
    yield;
  }
}

void start_generic()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  radio.use_ack = USE_ACK;
  radio.debug_mode = DEBUG_MODE;
}
void setup_sender()
{
#if ROLE == 1
  start_generic();
  bool startOK = radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  Serial.println("Im a Sender");
  Serial.print("start: ");
  Serial.println(startOK);
#endif
}
void setup_reciever()
{
#if ROLE == 0
  start_generic();
  bool startOK = radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
  Serial.println("Im Reciver");
  Serial.print("started: ");
  Serial.println(startOK);
#endif
}

void setup()
{
  setup_sender();
  setup_reciever();
}

void loop()
{
  recv_msg();
  radio.failDetect();
  radio.wellness_Watchdog();

  if (BOTH_TX_RX)
  {
    send_timely_msgs();
  }
  else
  {
#if ROLE == 1
    send_timely_msgs();
#endif
  }
}