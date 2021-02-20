// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~

/*sender*/
#if ROLE == 1     
const byte w_address = 1;
const byte r_address = 0;
const byte CE_PIN = 7;
const byte CSN_PIN = 8;

/*Receiver*/
#elif ROLE == 0
const byte w_address = 0;
const byte r_address = 1;
const byte CE_PIN = D4; /* ESP8266*/
const byte CSN_PIN = D2;
// const byte CE_PIN = 7;
// const byte CSN_PIN = 8;
#endif

myRF24 radio(CE_PIN, CSN_PIN);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~