// ~~~~~~~~~~~~ myRF24 lib ~~~~~~~~~~~~
#define DEBUG_MODE true;
#define USE_ACK false;
#define MAX_PAYLOAD_SIZE 150 //

#if ROLE == 1
const uint8_t w_address = 1;
const uint8_t r_address = 0;
const uint8_t CE_PIN = 9;
const uint8_t CSN_PIN = 10;

char *dev_name = "DEV1";
const int time_resend = 2500;

#elif ROLE == 0
const uint8_t w_address = 0;
const uint8_t r_address = 1;
const uint8_t CE_PIN = D2;
const uint8_t CSN_PIN = D4;
// const uint8_t CE_PIN = D2;
// const uint8_t CSN_PIN = D4;

char *dev_name = "DEV2";
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

void start_generic()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    radio.use_ack = USE_ACK;
    radio.debug_mode = DEBUG_MODE;
    delay(1000);
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