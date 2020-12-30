// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#if ROLE == 1     /*sender*/
#define USE_IOT 0 /*For devices NOT connected to WiFi*/
const byte w_address = 1;
const byte r_address = 0;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;
const char *dev_name = "ProMicro"; /*8 letters max*/

#elif ROLE == 0   /*Receiver*/
#define USE_IOT 0 /*For device connected to WiFi*/
const byte w_address = 0;
const byte r_address = 1;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;
// const byte CE_PIN = D4; /* ESP8266*/
// const byte CSN_PIN = D2;
const char *dev_name = "iot_port"; /*8 letters max*/
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~