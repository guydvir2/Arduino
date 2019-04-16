#include <myIOT.h>
#include <Arduino.h>
#include <FastLED.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/CorridorLEDs"

//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL    false
#define USE_WDT       true
#define USE_OTA       true
#define USE_IR_REMOTE false

#define NUM_LEDS 300 // <--- Verify if need to Change
#define DATA_PIN D4

//~~~~~~ Default Values ~~~~~~~
#define COLOR         1
#define LED_DELAY     10 // ms
#define BRIGHTNESS    20 // [0,100]
#define LED_DIRECTION 1  // [0,1]
#define PARAM_AMOUNT  5  // splitter 
#define MAX_BRIGHT    100
//~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define VER "Wemos.Mini.2.2"
//####################################################

//~~~~~~~~~~~~~~~~~~~~IR Remote ~~~~~~~~~~~~~~~~~~~~~
#if USE_IR_REMOTE
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t kRecvPin        = 14;
const uint32_t kBaudRate       = 115200;
const uint16_t kMinUnknownSize = 12;
unsigned long key_value        = 0;

IRrecv irrecv(kRecvPin);
decode_results results;
#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~LEDS defs ~~~~~~~~~~~~~~~~~~~~~

/*
   Color List:
   https://github.com/FastLED/FastLED/wiki/Pixel-reference
 */

//CRGB colors[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x008000, 0x0000FF, 0xFFD700, 0xFFDEAD};
//const char *color_names[] = {"Off", "White", "Red", "Green", "Blue", "Gold", "NavajoWhite"};

// CRGB colors[] = {0xF0F8FF, 0x9966CC, 0xFAEBD7, 0x00FFFF,
//                  0x7FFFD4, 0xF0FFFF, 0xF5F5DC, 0xFFE4C4, 0x000000, 0xFFEBCD, 0x0000FF, 0x8A2BE2,
//                  0xA52A2A, 0xDEB887, 0x5F9EA0, 0x7FFF00, 0xD2691E, 0xFF7F50, 0x6495ED, 0xFFF8DC,
//                  0xDC143C, 0x00FFFF, 0x00008B, 0x008B8B, 0xB8860B, 0xA9A9A9,  0xA9A9A9, 0x006400,
//                  0xBDB76B, 0x8B008B, 0x556B2F, 0xFF8C00, 0x9932CC, 0x8B0000, 0xE9967A, 0x8FBC8F,
//                  0x483D8B, 0x2F4F4F, 0x2F4F4F, 0x00CED1, 0x9400D3, 0xFF1493, 0x00BFFF, 0x696969,
//                  0x696969, 0x1E90FF, 0xB22222, 0xFFFAF0, 0x228B22, 0xFF00FF, 0xDCDCDC, 0xF8F8FF,
//                  0xFFD700, 0xDAA520, 0x808080, 0x808080, 0x008000, 0xADFF2F, 0xF0FFF0, 0xFF69B4,
//                  0xCD5C5C, 0x4B0082, 0xFFFFF0, 0xF0E68C, 0xE6E6FA, 0xFFF0F5, 0x7CFC00, 0xFFFACD,
//                  0xADD8E6, 0xF08080, 0xE0FFFF, 0xFAFAD2, 0x90EE90, 0xD3D3D3, 0xFFB6C1, 0xFFA07A,
//                  0x20B2AA, 0x87CEFA, 0x778899, 0x778899, 0xB0C4DE, 0xFFFFE0, 0x00FF00, 0x32CD32,
//                  0xFAF0E6, 0xFF00FF, 0x800000, 0x66CDAA, 0x0000CD, 0xBA55D3, 0x9370DB,  0x3CB371,
//                  0x7B68EE,  0x00FA9A,  0x48D1CC,  0xC71585, 0x191970,  0xF5FFFA,  0xFFE4E1,  0xFFE4B5,
//                  0xFFDEAD,  0x000080,  0xFDF5E6,  0x808000, 0x6B8E23,  0xFFA500,  0xFF4500,  0xDA70D6,
//                  0xEEE8AA,  0x98FB98,  0xAFEEEE,  0xDB7093, 0xFFEFD5,  0xFFDAB9,  0xCD853F,  0xFFC0CB,
//                  0xCC5533,  0xDDA0DD,  0xB0E0E6,  0x800080, 0xFF0000,  0xBC8F8F,  0x4169E1,  0x8B4513,
//                  0xFA8072,  0xF4A460,  0x2E8B57,  0xFFF5EE, 0xA0522D,  0xC0C0C0,  0x87CEEB,  0x6A5ACD,
//                  0x708090,  0x708090,  0xFFFAFA,  0x00FF7F, 0x4682B4,  0xD2B48C,  0x008080,  0xD8BFD8,
//                  0xFF6347,  0x40E0D0,  0xEE82EE,  0xF5DEB3, 0xFFFFFF,  0xF5F5F5,  0xFFFF00,  0x9ACD32,
//                  0xFFE42D,  0xFF9D2A};

CRGB colors[] = {0x000000,0xFFE4C4,0x9966CC,0xA52A2A,0x7FFF00,0xD2691E,
                 0xFFF8DC,0x00008B,0x8B008B,0x556B2F,0xFF8C00,0x483D8B,
                 0xFF1493, 0x228B22,0xFF00FF,0xADFF2F,0xE6E6FA,0xF08080};
const char *color_names[] = {"Black","Bisque","Amethyst","Brown","Chartreuse","Chocolate",
                             "Cornsilk","DarkBlue","DarkMagenta","DarkOliveGreen","DarkOrange","DarkSlateBlue",
                             "DeepPink","ForestGreen","Fuchsia","GreenYellow","Lavender","LightCoral"};

CRGB leds[NUM_LEDS];

const int tot_colors = int(sizeof(colors) / sizeof(colors[0]));
char parameters[PARAM_AMOUNT][4];
int param_def_values[] = {COLOR, BRIGHTNESS, LED_DELAY, LED_DIRECTION};
int partial_lit[] = {0, 90, 121, 208}; // trims led strip to partial lits. num are led ID's
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);


// ~~~~~~~~~~~~ LED Operations ~~~~~~~~~~~~~~~~~~~~~~
void turn_leds_on(int col_indx = COLOR, int bright_1 = BRIGHTNESS, int del_1 = LED_DELAY, bool dir_1 = LED_DIRECTION, int mode_1 = 0) {
        if ( col_indx <= tot_colors && bright_1 <= MAX_BRIGHT && del_1 <= 1000 && dir_1 <= 1) {
                FastLED.setBrightness(bright_1 * 255 / 100);
                if (dir_1 == true ) { // start to end
                        for (int i=0; i<NUM_LEDS; i++) {
                                if (mode_1 == 1) {
                                        if ((i >= partial_lit[0] && i <= partial_lit[1]) || (i >= partial_lit[2] && i <= partial_lit[3])) {
                                                leds[i] = colors[col_indx];
                                        }
                                }
                                else if (mode_1 == 2) {
                                        if ((i > partial_lit[1] && i < partial_lit[2])  || (i > partial_lit[3] && i <= NUM_LEDS )) {
                                                leds[i] = colors[col_indx];
                                        }
                                }
                                else if (mode_1 == 0) {
                                        leds[i] = colors[col_indx];
                                }
                                FastLED.show();
                                delay(del_1);
                        }
                }
                else if (dir_1 == false) { // end to start
                        for (int i = NUM_LEDS - 1; i >= 0; i = i - 1) {
                                leds[i] = colors[col_indx];
                                FastLED.show();
                                delay(del_1);
                        }
                }
        }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~ IR Commands ~~~~~~~
void inc_brightness() {
        int currentBright = atoi(parameters[1]) + 10;
        if ( currentBright <= MAX_BRIGHT) {
                sprintf(parameters[1], "%d", currentBright);
                turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), 1);
        }
}
void dec_brightness() {
        int currentBright = atoi(parameters[1]) - 10;
        if ( currentBright >= 10) {
                sprintf(parameters[1], "%d", currentBright);
                turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), 1);
        }
}
void change_color(int i) {
        sprintf(parameters[0], "%d", i);
        turn_leds_on(i);
}
void recvIRinputs() {
#if USE_IR_REMOTE
        if (irrecv.decode(&results)) {

                if (results.value == 0XFFFFFFFF)
                        results.value = key_value;

                switch (results.value) {
                case 0xFFA25D:
                        //Serial.println("CH-");
                        break;
                case 0xFF629D:
                        //Serial.println("CH");
                        break;
                case 0xFFE21D:
                        //Serial.println("CH+");
                        break;
                case 0xFF22DD:
                        //Serial.println("|<<");
                        break;
                case 0xFF02FD:
                        //Serial.println(">>|");
                        break;
                case 0xFFC23D:
                        //Serial.println(">|");
                        break;
                case 0xFFE01F:
                        //Serial.println("-");
                        dec_brightness();
                        break;
                case 0xFFA857:
                        //Serial.println("+");
                        inc_brightness();
                        break;
                case 0xFF906F:
                        //Serial.println("EQ");
                        break;
                case 0xFF6897:
                        //Serial.println("0");
                        turn_leds_on(0);
                        break;
                case 0xFF9867:
                        //Serial.println("100+");
                        sprintf(parameters[1], "%d", MAX_BRIGHT);
                        turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), 1);
                        break;
                case 0xFFB04F:
                        //Serial.println("200+");
                        break;
                case 0xFF30CF:
                        change_color(1);
                        break;
                case 0xFF18E7:
                        change_color(2);
                        break;
                case 0xFF7A85:
                        change_color(3);
                        break;
                case 0xFF10EF:
                        change_color(4);
                        break;
                case 0xFF38C7:
                        change_color(5);
                        break;
                case 0xFF5AA5:
                        //Serial.println("6");
                        break;
                case 0xFF42BD:
                        //Serial.println("7");
                        break;
                case 0xFF4AB5:
                        //Serial.println("8");
                        break;
                case 0xFF52AD:
                        //Serial.println("9");
                        break;
                }
                key_value = results.value;
                irrecv.resume();
        }
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~ Start Services ~~~~
void start_IR() {
#if USE_IR_REMOTE
#if DECODE_HASH
        // Ignore messages with less than minimum on or off pulses.
        irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
        irrecv.enableIRIn(); // Start the receiver
#endif
}
void start_LEDS() {
        for (int x = 0; x < PARAM_AMOUNT; x++) { //def values
                sprintf(parameters[x], "%d", param_def_values[x]);
        }
        FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~MQTT commands ~~~~~~~~~~~~~~~~
void splitter(char *inputstr) {
        char * pch;
        int i = 0;

        pch = strtok (inputstr, " ,.-");
        while (pch != NULL)
        {
                sprintf(parameters[i], "%s", pch);
                pch = strtok (NULL, " ,.-");
                i++;
                Serial.println(parameters[i - 1]);
        }
        //  update default values
        for (int n = i; n < PARAM_AMOUNT; n++) {
                sprintf(parameters[n], "%d", param_def_values[n]);
        }
}
void addiotnalMQTT(char incoming_msg[50]) {
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0) {
                // sprintf(msg, "Status: Color:[%s], Brightness:[%d]", color_names[atoi(parameters[0])], atoi(parameters[1]));
                sprintf(msg, "Status: Color:[%s], Brightness:[%d]", parameters[0], atoi(parameters[1]));
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], IRremote:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_IR_REMOTE);
                iot.pub_msg(msg);
        }
        else {
                splitter(incoming_msg);
                turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), atoi(parameters[2]), atoi(parameters[3]), atoi(parameters[4]));
                sprintf(msg, "TurnOn: Color:[%s], Brightness:[%d]", color_names[atoi(parameters[0])], atoi(parameters[1]));
                // sprintf(msg, "TurnOn: Color:[%s], Brightness:[%d]", parameters[0], atoi(parameters[1]));
                iot.pub_msg(msg);

        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void setup() {

        // ~~~~~~~~Start IOT sevices~~~~~~~~
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // ~~~~~~~~Start IR ~~~~~~~~                                                                                                                                                                                   nju
#if USE_IR_REMOTE
        start_IR();
#endif
        // ~~~~~~~~~~~~~~~~~~~~~~~~~

        // ~~~~~~~~Start LED ~~~~~~~~
        start_LEDS();
        turn_leds_on();
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~
}

void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        recvIRinputs();

        delay(100);
}
