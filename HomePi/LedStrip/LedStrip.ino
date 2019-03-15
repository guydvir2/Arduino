#include <myIOT.h>
#include <Arduino.h>
#include <FastLED.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/Kitchen_LedStrip"
//must be defined to use myIOT
#define ADD_MQTT_FUNC addiotnalMQTT
//~~~
#define USE_SERIAL false
#define USE_WDT true
#define USE_OTA true
#define USE_IR_REMOTE true

#define NUM_LEDS 150
#define DATA_PIN D4 // 7 for NodeMCU

#define COLOR 1
#define LED_DELAY 10
#define BRIGHTNESS  50
#define LED_DIRECTION 0
#define PARAM_AMOUNT 4

#define VER "Wemos.Mini.2.0"
//####################################################

//~~~~~~~~~~~~~~~~~~~~IR Remote ~~~~~~~~~~~~~~~~~~~~~
#if USE_IR_REMOTE
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t kRecvPin = 14; // D5 on a NodeMCU board
const uint32_t kBaudRate = 115200;
const uint16_t kMinUnknownSize = 12;
unsigned long key_value = 0;
#define MAX_BRIGHT 90

IRrecv irrecv(kRecvPin);//, kCaptureBufferSize, kTimeout, true);
decode_results results;
#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~LEDS defs ~~~~~~~~~~~~~~~~~~~~~
CRGB colors[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x008000, 0x0000FF, 0xFFD700, 0xFFDEAD}; // black, white,r,g,b
const char *color_names[] = {"Off", "White", "Red", "Green", "Blue", "Gold", "NavajoWhite"};
CRGB leds[NUM_LEDS];

const int tot_colors = int(sizeof(colors) / sizeof(colors[0]));
char parameters[PARAM_AMOUNT][4];
int param_def[] = {COLOR, BRIGHTNESS, LED_DELAY, LED_DIRECTION};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

myIOT iot(DEVICE_TOPIC);

void turn_leds_on(int col_indx = COLOR, int bright_1 = BRIGHTNESS, int del_1 = LED_DELAY, bool dir_1 = LED_DIRECTION) {
  if (USE_SERIAL) {
    Serial.print("Color: ");
    Serial.println(col_indx);
    Serial.print("Bright: ");
    Serial.println(bright_1);
    Serial.print("Delay: ");
    Serial.println(del_1);
    Serial.print("Direction: ");
    Serial.println(dir_1);
  }

  if ( col_indx <= tot_colors && bright_1 <= 100 && del_1 <= 1000 && dir_1 <= 1) {
    if (dir_1 == true ) { // start to end
      FastLED.setBrightness(bright_1 * 255 / 100);
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = colors[col_indx];
        FastLED.show();
        delay(del_1);
      }
    }

    else { // start to end
      for (int i = NUM_LEDS - 1; i >= 0; i = i - 1) {
        leds[i] = colors[col_indx];
        FastLED.setBrightness(bright_1 * 255 / 100);
        FastLED.show();
        delay(del_1);
      }
    }
  }
}

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
//  Serial.println(i);
  sprintf(parameters[0], "%d", i);
  turn_leds_on(i);
}

void splitter(char *inputstr) {
  char * pch;
  int i = 0;

  pch = strtok (inputstr, " ,.-");
  while (pch != NULL)
  {
    //    parameters[i] = pch;
    sprintf(parameters[i], "%s", pch);
    pch = strtok (NULL, " ,.-");
    i++;
  }
  //  update default values
  for (int n = i ; n <= PARAM_AMOUNT - 1; n++) {
    sprintf(parameters[n], "%d", param_def[n]);
  }
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
        break ;
      case 0xFFC23D:
        //Serial.println(">|");
        break ;
      case 0xFFE01F:
        //Serial.println("-");
        dec_brightness();
        break ;
      case 0xFFA857:
        //Serial.println("+");
        inc_brightness();
        break ;
      case 0xFF906F:
        //Serial.println("EQ");
        break ;
      case 0xFF6897:
        //Serial.println("0");
        turn_leds_on(0);
        break ;
      case 0xFF9867:
        //Serial.println("100+");
        sprintf(parameters[1], "%d", MAX_BRIGHT);
        turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), 1);
        break ;
      case 0xFFB04F:
        //Serial.println("200+");
        break ;
      case 0xFF30CF:
        change_color(1);
        break ;
      case 0xFF18E7:
        change_color(2);
        break ;
      case 0xFF7A85:
        change_color(3);
        break ;
      case 0xFF10EF:
        change_color(4);
        break ;
      case 0xFF38C7:
        change_color(5);
        break ;
      case 0xFF5AA5:
        //Serial.println("6");
        break ;
      case 0xFF42BD:
        //Serial.println("7");
        break ;
      case 0xFF4AB5:
        //Serial.println("8");
        break ;
      case 0xFF52AD:
        //Serial.println("9");
        break ;
    }
    key_value = results.value;
    irrecv.resume();
  }
#endif
}

void start_IR() {

#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
  irrecv.enableIRIn();  // Start the receiver
}

void start_LEDS() {
  for (int x = 0 ; x <= PARAM_AMOUNT - 1; x++) { //def values
    sprintf(parameters[x], "%d", param_def[x]);
  }
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  turn_leds_on(); // run on def values
}

void setup() {
  // ~~~~~~~~Start IOT sevices~~~~~~~~
  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.start_services(ADD_MQTT_FUNC);
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // ~~~~~~~~Start IR ~~~~~~~~
#if USE_IR_REMOTE
  start_IR();
#endif
  // ~~~~~~~~~~~~~~~~~~~~~~~~~

  // ~~~~~~~~Start LED ~~~~~~~~
  start_LEDS();
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~
}

void addiotnalMQTT(char incoming_msg[50]) {
  char state[5];
  char state2[5];
  char msg[100];
  char msg2[100];

  if (strcmp(incoming_msg, "status") == 0) {
    sprintf(msg, "Status: Color:[%s], Brightness:[%d]", color_names[atoi(parameters[0])], atoi(parameters[1]));
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver") == 0 ) {
    sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], IRremote:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_IR_REMOTE);
    iot.pub_msg(msg);
  }
  else {
    splitter(incoming_msg);

    turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), atoi(parameters[2]), atoi(parameters[3]));
    sprintf(msg, "TurnOn: Color:[%s], Brightness:[%d]", color_names[atoi(parameters[0])], atoi(parameters[1]));
    iot.pub_msg(msg);

  }
}

void loop() {
  iot.looper(); // check wifi, mqtt, wdt
  recvIRinputs();

  delay(100);
}
