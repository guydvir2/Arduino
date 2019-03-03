#include <myIOT.h>
#include <Arduino.h>
#include <FastLED.h>

//####################################################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/LedStrip2"
//must be defined to use myIOT
#define ADD_MQTT_FUNC addiotnalMQTT
//~~~
#define USE_SERIAL true
#define USE_WDT true
#define USE_OTA true

#define NUM_LEDS 150
#define DATA_PIN D4

#define COLOR 1
#define LED_DELAY 20
#define BRIGHTNESS  50
#define LED_DIRECTION 0
#define WAVE false
#define PARAM_AMOUNT 4

#define VER "Wemos.Mini.1.3"
//####################################################


CRGB colors[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x008000, 0x0000FF, 0xFFD700, 0xFFDEAD}; // black, white,r,g,b
const char *color_names[] = {"Off", "White", "Red", "Green", "Blue", "Gold", "NavajoWhite"};
CRGB leds[NUM_LEDS];
myIOT iot(DEVICE_TOPIC);

const int tot_colors = int(sizeof(colors) / sizeof(colors[0]));
char parameters[PARAM_AMOUNT][4];
int param_def[] = {COLOR, BRIGHTNESS, LED_DELAY, LED_DIRECTION};

void turn_leds_on(int col_indx = COLOR, int bright_1 = BRIGHTNESS, int del_1 = LED_DELAY, bool dir_1 = LED_DIRECTION) {
  Serial.print("Color: ");
  Serial.println(col_indx);
  Serial.print("Bright: ");
  Serial.println(bright_1);
  Serial.print("Delay: ");
  Serial.println(del_1);
  Serial.print("Direction: ");
  Serial.println(dir_1);

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

void setup() {
  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.start_services(ADD_MQTT_FUNC); // additinalMQTTfucntion, ssid,pswd,mqttuser,mqtt_pswd,broker

  for (int x = 0 ; x <= PARAM_AMOUNT - 1; x++) { //def values
    sprintf(parameters[x], "%d", param_def[x]);
  }
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  turn_leds_on(); // run on def values
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
    sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL);
    iot.pub_msg(msg);
  }
  else {
    splitter(incoming_msg);

    //    if ( atoi(parameters[0]) <= tot_colors - 1 && isDigit(*parameters[0])) { // color num
    //
    //      if (atoi(parameters[1]) <= 100 && isDigit(*parameters[1])) { //brightness
    //        if (atoi(parameters[2]) <= 100 && isDigit(*parameters[2])) { //delay between leds
    //          if (atoi(parameters[3]) <= 1 && isDigit(*parameters[3])) { //direction
    Serial.println("Parameters to sent to fuction");
    Serial.println(parameters[0]);
    Serial.println(parameters[1]);
    Serial.println(parameters[2]);
    Serial.println(parameters[3]);

    turn_leds_on(atoi(parameters[0]), atoi(parameters[1]), atoi(parameters[2]), atoi(parameters[3]));
    sprintf(msg, "TurnOn: Color:[%s], Brightness:[%d]", color_names[atoi(parameters[0])], atoi(parameters[1]));
    iot.pub_msg(msg);
    //          }
    //        }
    //      }
    //    }
    //  }
  }
}


void loop() {
  iot.looper(); // check wifi, mqtt, wdt

  delay(100);
}
