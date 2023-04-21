#include <myRF24.h>

#if isESP8266
#define _CE_PIN D4
#define _CSN_PIN D2
#else
#define _CE_PIN 9
#define _CSN_PIN 10
#endif



// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define USE_ACK false;

#if ROLE == 1
char *dev_name = "DEV1";
const uint8_t CE_PIN = _CE_PIN;
const uint8_t CSN_PIN = _CSN_PIN;
const uint8_t w_address = 1;
const uint8_t r_address = 0;
const int time_resend = 2500;

#elif ROLE == 0
char *dev_name = "DEV0";
const uint8_t CE_PIN = _CE_PIN;
const uint8_t CSN_PIN = _CSN_PIN;
const uint8_t w_address = 0;
const uint8_t r_address = 1;
const int time_resend = 23456;
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

extern void incomeMSG_cb(char *_from, char *_msg);

void RF24_Rx_looper()
{
    char from[10];
    char inmsg_buff[MAX_PAYLOAD_SIZE];

    const uint8_t delay_read = 100;
    if (radio.RFread(inmsg_buff, from, delay_read))
    {
        incomeMSG_cb(from, inmsg_buff);
    }
}

void RF24_init()
{
    char a[20];
    const char *b[]={"Reciever","Sender"};
    radio.use_ack = USE_ACK;
    radio.debug_mode = DEBUG_MODE;
    delay(200);
    bool startOK = radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
    sprintf(a, "RF24 init [%s]", startOK ? "OK" : "FAIL");
    Serial.print(a);
    Serial.print("\t ROLE:");
    Serial.print(b[ROLE]);
    Serial.print("\t Name:");
    Serial.println(dev_name);
}

