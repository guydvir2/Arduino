/*

 ** OLED clock by guyDvir 05/2019 **

   Using NODEMCU 8266 to display clock using NTP service, and
   display it on OLED display.

   A press button switches between 2 modes:
   1) Clock display( update using NTP services)
   2) Stopwatch display

   Short press switches between 2 modes,
   while 3 seconds press resets stopwatch ( restart count )

 */

#include <myIOT.h>
#include <Arduino.h>

//~~~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 32    // 32 2rows or 64 4 rows
#define OLED_RESET    LED_BUILTIN


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~ myIOT  ~~~~~~~~~~~~~~~~~~~~~
#define DEVICE_TOPIC        "HomePi/Dvir/OLED_Clock"
#define ADD_MQTT_FUNC       addiotnalMQTT
#define USE_SERIAL          true
#define USE_WDT             true
#define USE_OTA             true

#define VER                 "NodeMCU_V1.1"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define PRESSED             LOW
#define SYS_STATE_STWATCH   true

unsigned long start_watch=0;
byte days   =0;
byte hours  =0;
byte minutes=0;
byte seconds=0;
int micross =0;


const int buttonPin  = D4;
bool systemState     = false; // clock or stopwatch


// Start services ~~~~~~~~~~~~~~~~~~~~~~~~~~
myIOT iot(DEVICE_TOPIC);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void addiotnalMQTT(char *incoming_msg) {
        char msg[50];

        if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: [%s] Mode",systemState ? "StopWatch" : "Clock");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "start") == 0) {
                sprintf(msg, "StopWatch: [start]");
                start_stopwatch();
                systemState = SYS_STATE_STWATCH;
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "clock") == 0) {
                sprintf(msg, "Mode: [Clock]");
                systemState = !SYS_STATE_STWATCH;
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "stopwatch") == 0) {
                sprintf(msg, "Mode: [StopWatch]");
                systemState = SYS_STATE_STWATCH;
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: [status, start, clock, stopwatch, help] , [ver, boot, reset, ip, ota]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d]", VER,iot.ver, USE_WDT, USE_OTA, USE_SERIAL);
                iot.pub_msg(msg);
        }
}
void put_text(byte size = 1, byte x = 0, byte y = 0, char *text = "") {
        display.setTextSize(size);
        display.setTextColor(WHITE);
        display.setCursor(y, x);
        display.print(text);
}
void center_text(int i, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte shift = 0) {
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(i);
        display.setTextColor(WHITE);
        byte line_space = pow(2, (2 + i));

        for (int n = 0; n < 4; n++) {
                if (strcmp(Lines[n], "") != 0) {
                        int strLength = strlen(Lines[n]);
                        display.setCursor((ceil)((21 / i - strLength) / 2 * (128 / (21 / i))) + shift,  line_space * n);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void show_clock(){
        char a[12];
        char b[12];
        iot.return_clock(a);
        iot.return_date(b);
        center_text(2,a, b);
}
void start_stopwatch(){
        start_watch = millis();
}
void calc_stopwatch(){
        unsigned int time_delta=millis()-start_watch;
        days    = (int)(time_delta/(1000*60*60*24));
        hours   = (int)((time_delta -days*1000*60*60*24)/(1000*60*60));
        minutes = (int)((time_delta -days*1000*60*60*24- hours*1000*60*60)/(1000*60));
        seconds = (int)((time_delta -days*1000*60*60*24- hours*1000*60*60- minutes*1000*60)/1000);
        micross  = (int)((time_delta -days*1000*60*60*24- hours*1000*60*60- minutes*1000*60- seconds*1000)/100);
}
void show_stopwatch(){
        char day_char[12];
        char stopwatch_char[50];

        calc_stopwatch();

        sprintf(day_char,"%02d days",days);
        sprintf(stopwatch_char,"%02d:%02d:%02d:%01d",hours, minutes, seconds,micross);

        center_text(2,day_char, stopwatch_char);
}
void setup() {
        pinMode(buttonPin, INPUT_PULLUP);

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);

        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        /*  size: 1 = 21X4
           /   size: 2 = 10X2
            size: 4 = 5 X1
         */
}
void loop() {
        if(digitalRead(buttonPin) == PRESSED ) {
                delay(50); // debounce
                if (digitalRead(buttonPin) == PRESSED) {
                        unsigned long presslength = millis();
                        systemState = !systemState; // change state
                        if (systemState == SYS_STATE_STWATCH) { //true for stopwatch mode
                                //         while (digitalRead(buttonPin) == PRESSED) {
                                //                 delay(50);
                                //         }
                                //         if (millis()-presslength > 3000) { // reset stopwatch
                                //                 iot.pub_msg("Mode: [StopWatch], Counter: [Reset]");
                                //                 start_stopwatch();
                                //         }
                                //         else{
                                //                 iot.pub_msg("Mode: [StopWatch], Counter: [Continue]");
                                //         }
                                // }
                                iot.pub_msg("Mode: [StopWatch]");
                        }
                        else{
                                iot.pub_msg("Mode: [Clock]");
                        }
                        delay(300);
                }
        }

        if (systemState == true) {
                show_stopwatch();
        }
        else {
                show_clock();
        }
        iot.looper();
}
