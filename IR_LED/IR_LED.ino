#include <myIOT.h>
#include <FastLED.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER              "WEMOS_1.2"
#define USE_IR_REMOTE    true
#define COLOR            1
#define LED_DELAY        2 // ms
#define BRIGHTNESS       5 // [0,100]
#define LED_DIRECTION    true  // [0,1]
#define MAX_BRIGHT       100
#define MIN_BRIGHT       5
#define JUMP_BRIGHT      10
#define IR_SENSOR_PIN    D5


//~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  ~~~~~~ LEDS ~~~~~~~~~~~~
#define NUM_LEDS         150
#define LED_DATA_PIN     D4
#define CLOCK_PIN        13

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false // Serial Monitor
#define USE_WDT          true  // watchDog resets
#define USE_OTA          true  // OTA updates
#define USE_RESETKEEPER  false // detect quick reboot and real reboots
#define USE_FAILNTP      true  // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC    "kidsColorLEDs"
#define MQTT_PREFIX     "myHome"
#define MQTT_GROUP      "intLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

//~~~~~~~ IR Remote ~~~~~~~~
#if USE_IR_REMOTE
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t kRecvPin        = IR_SENSOR_PIN;
const uint32_t kBaudRate       = 115200;
const uint16_t kMinUnknownSize = 12;
unsigned long key_value        = 0;

IRrecv irrecv(kRecvPin);
decode_results results;

#endif

//  ~~~~~ LEDS ~~~~~
CRGB leds[NUM_LEDS];
byte ledBrightness = MAX_BRIGHT;
byte ledColor      = COLOR;
bool Cylon_flag    = false;

//~~~~~~~~~~~~~~~~~~~~LEDS defs ~~~~~~~~~~~~~~~~~~~~~

/*
   Color List:
   https://github.com/FastLED/FastLED/wiki/Pixel-reference
 */

CRGB colors[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x008000, 0x0000FF,
                 0xF0F8FF, 0x9966CC,0xFAEBD7, 0x7FFF00,0x8B0000,
                 0x696969,0xFF1493,0x90EE90,0x87CEEB,0xFF6347};

const char *color_names[] = {"Black", "White", "Red", "Green", "Blue",
                             "AliceBlue","Amethyst","AntiqueWhite","Chartreuse","DarkRed"
                             "DimGray","DeepPink","LightGreen","SkyBlue","Tomato"};

const int tot_colors = int(sizeof(colors) / sizeof(colors[0]));

// #############################################################################


void startIOTservices() {
        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}


//~~~~~~~Run IR Remote ~~~~~~~~
void recvIRinputs() {
#if USE_IR_REMOTE
        char msg[50];

        if (irrecv.decode(&results)) {

                if (results.value == 0XFFFFFFFF)
                        results.value = key_value;
                char msg[50];

                switch (results.value) {
                case 0xFFA25D:
                        // Serial.println("CH-");
                        break;
                case 0xFF629D:
                        //Serial.println("CH");
                        iot.sendReset("RemoteControl");
                        break;
                case 0xFFE21D:
                        //Serial.println("CH+");
                        break;
                case 0xFF22DD:
                        //Serial.println("|<<");
                        chng_color(-1);
                        break;
                case 0xFF02FD:
                        //Serial.println(">>|");
                        chng_color(1);
                        break;
                case 0xFFC23D:
                        //Serial.println(">|");
                        break;
                case 0xFFE01F:
                        //Serial.println("-");
                        chng_brightness(-JUMP_BRIGHT);
                        break;
                case 0xFFA857:
                        //Serial.println("+");
                        chng_brightness(JUMP_BRIGHT);
                        break;
                case 0xFF906F:
                        //Serial.println("EQ");
                        break;
                case 0xFF6897:
                        //Serial.println("0");
                        set_color(0);
                        break;
                case 0xFF9867:
                        //Serial.println("100+");
                        turn_leds_off();
                        LEDS.setBrightness(MAX_BRIGHT);
                        Cylon_flag = !Cylon_flag;
                        sprintf(msg, "Color: Changed to [Cylon]");
                        iot.pub_msg(msg);
                        break;
                case 0xFFB04F:
                        //Serial.println("200+");
                        break;
                case 0xFF30CF:
                        set_color(1);
                        break;
                case 0xFF18E7:
                        set_color(2);
                        break;
                case 0xFF7A85:
                        set_color(3);
                        break;
                case 0xFF10EF:
                        set_color(4);
                        break;
                case 0xFF38C7:
                        set_color(5);
                        break;
                case 0xFF5AA5:
                        set_color(6);
                        break;
                case 0xFF42BD:
                        set_color(7);
                        break;
                case 0xFF4AB5:
                        set_color(8);
                        break;
                case 0xFF52AD:
                        set_color(9);
                        break;
                }
                key_value = results.value;
                irrecv.resume();
        }
#endif
}
void start_IR() {
#if USE_IR_REMOTE
#if DECODE_HASH
        // Ignore messages with less than minimum on or off pulses.
        irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
        irrecv.enableIRIn(); // Start the receiver
#endif
}


// ~~~~~~ LED Operations ~~~~~~~~~~
void turn_leds_off() {

        for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = colors[0];
                delay(LED_DELAY);
                FastLED.show();
        }

        if (Cylon_flag) {
                Cylon_flag = false;
        }
}

void turn_leds_on(int col_indx = COLOR, int bright_1 = BRIGHTNESS, int del_1 = LED_DELAY, bool dir_1 = LED_DIRECTION) {

        if ( col_indx <= tot_colors && bright_1 <= MAX_BRIGHT && del_1 <= 1000 && dir_1 <= 1) {
                FastLED.setBrightness((bright_1*255/100));
                if (dir_1 == true ) { // start to end
                        for (int i = 0; i < NUM_LEDS; i++) {
                                leds[i] = colors[col_indx];
                                // FastLED.show();
                                // delay(del_1);
                        }
                }
                else { // end to start
                        for (int i = NUM_LEDS - 1; i >= 0; i = i - 1) {
                                leds[i] = colors[col_indx];
                                // FastLED.show();
                                // delay(del_1);
                        }
                }
        }
        FastLED.show();
}
void set_bright(byte val){
        char msg[50];

        if (val != ledBrightness) {
                ledBrightness = val;
                turn_leds_on(ledColor, ledBrightness, 0);

                sprintf(msg, "Brightness: Changed to [%d], range:[%d/%d]",val,MIN_BRIGHT, MAX_BRIGHT);
                iot.pub_msg(msg);
        }
}
void set_color(byte col_i){
        char msg[50];

        if ( col_i < tot_colors - 1 && col_i >=0) {
                ledColor = col_i;
                turn_leds_off();
                turn_leds_on(ledColor, ledBrightness);

                sprintf(msg, "Color: Changed to [%s] out of [%d] colors Available",color_names[col_i], tot_colors);
                iot.pub_msg(msg);
        }
}
void chng_brightness(int val) {
        // if (ledBrightness != MIN_BRIGHT && ledBrightness != MAX_BRIGHT) {
        if (ledBrightness + val <= MAX_BRIGHT && ledBrightness + val>= MIN_BRIGHT ) {
                ledBrightness += val;
        }
        else if (ledBrightness + val > MAX_BRIGHT) {
                ledBrightness = MAX_BRIGHT;
        }
        else if (ledBrightness + val < MIN_BRIGHT) {
                ledBrightness = MIN_BRIGHT;
        }
        set_bright(ledBrightness);
}
void chng_color(int col_i) {
        if ( ledColor + col_i < tot_colors - 1 && ledColor + col_i >=0) {
                ledColor += col_i;
                set_color(ledColor);
        }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~


void start_LEDS(){
        LEDS.addLeds<WS2812,LED_DATA_PIN,RGB>(leds,NUM_LEDS);
        LEDS.setBrightness(MAX_BRIGHT/2);
}
void fadeall() {
        for(int i = 0; i < NUM_LEDS; i++) {
                leds[i].nscale8(250);
        }
}
void LEDS_looper(){
        static uint8_t hue = 0;
        // uint8_t hue = 0;

        // First slide the led in one direction
        for(int i = 0; i < NUM_LEDS; i++) {
                // Set the i'th led to red
                leds[i] = CHSV(hue++, 255, 255);
                // Show the leds
                FastLED.show();
                // now that we've shown the leds, reset the i'th led to black
                // leds[i] = CRGB::Black;
                fadeall();
                // Wait a little bit before we loop around and do it again
                delay(10);
        }

        // Now go in the other direction.
        for(int i = (NUM_LEDS)-1; i >= 0; i--) {
                // Set the i'th led to red
                leds[i] = CHSV(hue++, 255, 255);
                // Show the leds
                FastLED.show();
                // now that we've shown the leds, reset the i'th led to black
                // leds[i] = CRGB::Black;
                fadeall();
                // Wait a little bit before we loop around and do it again
                delay(10);
        }
}
void addiotnalMQTT(char *incoming_msg) {
        char msg[150];
        char msg2[20];

        if      (strcmp(incoming_msg, "status") == 0) {
                if (Cylon_flag == false) {
                        sprintf(msg, "Status: Color [%s], Brightness [%d]", color_names[ledColor], ledBrightness);
                        iot.pub_msg(msg);
                }
                else{
                        sprintf(msg, "Status: Color [%s], Brightness [%s]", "Cylon Palette", ledBrightness);
                        iot.pub_msg(msg);
                }
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [on, off, bright, color]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "off") == 0) {
                turn_leds_off();
                sprintf(msg, "MQTT: Turn [OFF]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "on") == 0) {
                turn_leds_on();
                sprintf(msg, "MQTT: Turn [ON]");
                iot.pub_msg(msg);
        }
        else {
                iot.inline_read(incoming_msg);

                if (strcmp(iot.inline_param[0], "bright") == 0 ) {
                        set_bright(atoi(iot.inline_param[1]));
                }
                else if (strcmp(iot.inline_param[0], "color") == 0 ) {
                        set_color(atoi(iot.inline_param[1]));
                }
        }
}

// ########################### END ADDITIONAL SERVICE ##########################


void setup() {
        startIOTservices();

        #if USE_IR_REMOTE
        start_IR();
        #endif

        start_LEDS();
        turn_leds_on();
}
void loop() {
        iot.looper();

        #if USE_IR_REMOTE
        recvIRinputs(); // IR signals
        #endif

        if (Cylon_flag) {
                LEDS_looper();
        }

        delay(100);
}
