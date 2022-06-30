#include <myRF24.h>

#if isESP8266
#define _CE_PIN D4
#define _CSN_PIN D2
#else
#define _CE_PIN 9
#define _CSN_PIN 10
#endif

// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#if ROLE == 1
char *dev_name = "DEV1";
const uint8_t w_address = 1;
const uint8_t r_address = 0;

#elif ROLE == 0
char *dev_name = "HUB";
const uint8_t w_address = 0;
const uint8_t r_address = 1;
#endif

#define USE_ACK false;

const uint8_t CE_PIN = _CE_PIN;
const uint8_t CSN_PIN = _CSN_PIN;

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

unsigned int totRx = 0;
// unsigned int totTx = 0;
// uint8_t Tx_fails_counter = 0;

void incomeMSG_cb(char *from, char *msg)
{
    char qwerty[MAX_PAYLOAD_SIZE];
    sprintf(qwerty, "Rx[#%d]: from[%s]; msg[%s]", ++totRx, from, msg);
    Serial.println(qwerty);
}
void send_RF_msg(char *msg)
{
    // static unsigned int counter = 0;

    // sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, Tx_fails_counter, millis() / 1000);
    // sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, Tx_fails_counter, millis() / 1000);

    bool sent_OK = radio.RFwrite(msg);
    if (!sent_OK)
    {
        // sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, ++Tx_fails_counter);
        Serial.println("failed");
    }
    //   char qwerty[60];
    //   static unsigned int counter = 0;
    //   static unsigned long last_msg = 0;

    //   int randnum = random(50, 150);

    //   while (millis() - last_msg > randnum)
    //   {
    //     last_msg = millis();
    //     sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, Tx_fails_counter, millis() / 1000);
    //     bool sent_OK = radio.RFwrite(qwerty);
    //     if (!sent_OK)
    //     {
    //       sprintf(qwerty, "Tx[#%d /failed:#%d]>> MSG:%d[sec]", ++totTx, ++Tx_fails_counter);
    //       Serial.println(qwerty);
    //     }
    // #if DEBUG_MODE && sent_OK
    //     Serial.println(qwerty);
    // #endif
    //   }
}
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
    char *b[]={"Receiver","Sender"};
    radio.use_ack = USE_ACK;
    radio.debug_mode = DEBUG_MODE;
    // delay(200);
    bool startOK = radio.startRF24(w_address, r_address, dev_name, RF24_PA_MIN, RF24_1MBPS, 1);
    sprintf(a, "RF24 started [%s]", startOK ? "OK" : "FAIL");
    Serial.print(a);
    Serial.print("\t ROLE:");
    Serial.print(b[ROLE]);
    Serial.print("\t Name:");
    Serial.println(dev_name);
}
