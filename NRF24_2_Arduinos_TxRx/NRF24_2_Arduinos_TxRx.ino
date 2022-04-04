#include <myRF24.h>
#include <ArduinoJson.h>

#define ROLE 1 /* 0:Reciever (Arduino with RF24 log range anttenna) */
               /* 1: Sender  (Arduino with RF24 log range anttenna)*/
#define PRINT_MESSAGES true

// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define DEBUG_MODE true;
#define USE_ACK false;
#if ROLE == 1
const uint8_t w_address = 1;
const uint8_t r_address = 0;
const uint8_t CE_PIN = 9;
const uint8_t CSN_PIN = 10;

#elif ROLE == 0
const uint8_t w_address = 0;
const uint8_t r_address = 1;
const uint8_t CE_PIN = 9;
const uint8_t CSN_PIN = 10;
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if ROLE == 1
char *dev_name = "Rdu_1";
const int time_resend = 250;

#elif ROLE == 0
const char *dev_name = "Rdu_0";
const int time_resend = 23456;
#endif

void recvMSG_cb(char *incomeMSG)
{
  Serial.println(incomeMSG);
}
void send_timely_msgs()
{
  static long last_msg = 0;
  static unsigned int counter = 0;
  while (millis() - last_msg > time_resend)
  {
    char qwerty[60];
    sprintf(qwerty, "msg #%d: time: %s", counter++, "ABCDEFGRHNRTEHNREWGTBGBSFGB");
    last_msg = millis();
    radio.RFwrite(qwerty);
  }
}
void recv_msg()
{
  char inmsg_buff[250];
  const uint8_t delay_read = 100;
  if (radio.RFread2(inmsg_buff, delay_read))
  {
    recvMSG_cb(inmsg_buff);
    // answer_incoming(inmsg_buff);
  }
}
 
void start_generic()
{
  Serial.begin(115200);
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
  // radio.failDetect();
#if ROLE == 1
  send_timely_msgs();
#endif
  // radio.wellness_Watchdog();
}
