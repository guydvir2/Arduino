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
#include <myJSON.h>
#include <Arduino.h>
#include <TimeLib.h>

//~~~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       32    // 32 2rows or 64 4 rows
#define OLED_RESET          LED_BUILTIN
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~ myIOT  ~~~~~~~~~~~~~~~~~~~~~
#define DEVICE_TOPIC        "HomePi/Dvir/OLED_Clock2"
#define ADD_MQTT_FUNC       addiotnalMQTT
#define USE_SERIAL          true
#define USE_WDT             true
#define USE_OTA             true

#define VER                 "Wemos_V1.4"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~ myJSON  ~~~~~~~~~~~~~~~~~~~~~
#define jfile "myfile.json"
#define STATE_KEY           "state"
#define BOOT_KEY            "BootTime"
#define STPWATCH_KEY        "stopWatch_clock"

// Read from FLASH
unsigned long savedBoot      = 0;
unsigned long savedStopwatch = 0;
bool savedSystemState        = false;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define buttonPin           D4
#define PRESSED             LOW
#define SYS_STATE_STWATCH   true


byte days    = 0;
byte hours   = 0;
byte minutes = 0;
byte seconds = 0;
int micross  = 0;
unsigned long start_watch    = 0;
unsigned long local_bootTime = 0;
int ignore_lastboot_interval = 30; //sec
bool systemState     = false; // display clock or stopwatch





// ~~~~~~~~~~~~~~Start services ~~~~~~~~~~~~
myIOT iot(DEVICE_TOPIC);
myJSON json(jfile, true); // <----
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void addiotnalMQTT(char *incoming_msg) {
        char msg[50];

        if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: [%s] Mode", systemState ? "StopWatch" : "Clock");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "start") == 0) {
                sprintf(msg, "StopWatch: [start]");
                // start_stopwatch(now());
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "clock") == 0) {
                sprintf(msg, "Mode: [Clock]");
                systemState = !SYS_STATE_STWATCH;
                json.setValue(STATE_KEY,0);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "stopwatch") == 0) {
                sprintf(msg, "Mode: [StopWatch]");
                systemState = SYS_STATE_STWATCH;
                json.setValue(STATE_KEY,1);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: [status, start, clock, stopwatch, uptime, help] , [ver, boot, reset, ip, ota]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "uptime") == 0 ) {
                sprintf(msg, "UpTime: %02d days  %02d:%02d:%02d.%01d", days, hours, minutes, seconds, micross);
                iot.pub_msg(msg);
        }
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
void show_clock() {
        char a[12];
        char b[12];
        iot.return_clock(a);
        iot.return_date(b);
        center_text(2, a, b);
}

void start_stopwatch(unsigned long defVal=local_bootTime) {
        char clock[20];

        start_watch = defVal;
        sprintf(clock,"%d",start_watch);
        json.setValue(STATE_KEY, 1);
        json.setValue(STPWATCH_KEY, clock);
        systemState = SYS_STATE_STWATCH;
}
void stop_stopwatch() {
        json.setValue(STATE_KEY, 0);
        json.setValue(STPWATCH_KEY, "0");
        systemState = !SYS_STATE_STWATCH;
}

void calc_stopwatch() {
        unsigned long time_delta = now() - start_watch; //millis() - start_watch;
        //  Serial.println(now());
        //  Serial.println(start_watch);
        //  days    = (int)(time_delta / (1000 * 60 * 60 * 24));
        //  hours   = (int)((time_delta - days * 1000 * 60 * 60 * 24) / (1000 * 60 * 60));
        //  minutes = (int)((time_delta - days * 1000 * 60 * 60 * 24 - hours * 1000 * 60 * 60) / (1000 * 60));
        //  seconds = (int)((time_delta - days * 1000 * 60 * 60 * 24 - hours * 1000 * 60 * 60 - minutes * 1000 * 60) / 1000);
        //  micross  = (int)((time_delta - days * 1000 * 60 * 60 * 24 - hours * 1000 * 60 * 60 - minutes * 1000 * 60 - seconds * 1000) / 100);

        int sec2minutes = 60;
        int sec2hours   = (sec2minutes*60);
        int sec2days    = (sec2hours*24);
        int sec2years   = (sec2days*365);

        days    = (int)(time_delta/sec2days);
        hours   = (int)((time_delta - days*sec2days) / sec2hours);
        minutes = (int)((time_delta - days*sec2days - hours*sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days*sec2days - hours*sec2hours - minutes*sec2minutes);
}
void show_stopwatch() {
        char day_char[12];
        char stopwatch_char[50];

        calc_stopwatch();
        sprintf(day_char, "%02d days", days);
        //  sprintf(stopwatch_char, "%02d:%02d:%02d.%01d", hours, minutes, seconds, micross);
        sprintf(stopwatch_char, "%02d:%02d:%02d", hours, minutes, seconds);
        center_text(2, day_char, stopwatch_char);
}

bool checkBoot_timestamp() {
        const char *tmp;
        if (json.getCharValue(BOOT_KEY,tmp)) {
                Serial.println("Boot retrieved");
                Serial.println(tmp);
                // savedBoot = atoi(tmp);
        }
        else{
                json.setValue(BOOT_KEY,"0");
                Serial.println("Boot default saved");
        }

        //   long currentBootTime = now();
        //   unsigned long stopwatchVal = load_stopwatchValue();
        //   char temp[15];
        //
        //   if (year(currentBootTime) != 1970) { //NTP update succeeded
        //           if (json.getCharValue("bootTime",savedBoot)) { // previous value not saved on flash
        //                   if (currentBootTime - atoi(savedBoot) > ignore_lastboot_interval ) {
        //                           sprintf(temp, "%d", currentBootTime);
        //                           json.setValue("bootTime", temp);
        //                           Serial.print("Time updated");
        //                           local_bootTime = currentBootTime; // take clock of current boot
        //                   }
        //                   else if ( currentBootTime - atoi(savedBoot) <= ignore_lastboot_interval && stopwatchVal > 0) {
        //                           Serial.println("No time update");
        //                           local_bootTime = atoi(savedBoot); // take clock of last boot
        //                           start_stopwatch()
        //                   }
        //                   return 1;
        //           }
        //           else {
        //                   local_bootTime = currentBootTime;
        //                   return 0;
        //           }
        //   }
        // }
}

void load_systemState(){
        if (json.getINTValue(STATE_KEY, savedSystemState)) {
                Serial.print("SystemState retrieved: ");
                Serial.println(savedSystemState);
        }
        else{
                // json.setValue(STATE_KEY,0);
                // Serial.println("State default saved");
                systemState= false; // show watch
        }
}

void load_stopwatchValue(){
        const char *stopwatch;
        if(json.getCharValue(STPWATCH_KEY,stopwatch)) {
                if (stopwatch!=NULL) {
                        savedStopwatch = atoi(stopwatch);
                        Serial.print("Stopwatch retrieved: ");
                        Serial.println(savedStopwatch);
                }
                else{
                        json.setValue(STPWATCH_KEY,"0");
                        Serial.println("State default saved");
                }
        }
        else{
                json.setValue(STPWATCH_KEY,"0");
                Serial.println("State default saved");
        }
}

void setup() {
        pinMode(buttonPin, INPUT_PULLUP);

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);

        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        json.setValue(STATE_KEY,1);

        // load_systemState();
        load_stopwatchValue();
        // checkBoot_timestamp();


        /*  size: 1 = 21X4
           /   size: 2 = 10X2
            size: 4 = 5 X1
         */

}
void loop() {
        if (digitalRead(buttonPin) == PRESSED ) {
                delay(50); // debounce
                if (digitalRead(buttonPin) == PRESSED) {
                        unsigned long presslength = millis();
                        systemState = !systemState; // change state
                        if (systemState == SYS_STATE_STWATCH) { //true for stopwatch mode
                                while (digitalRead(buttonPin) == PRESSED) {
                                        delay(50);
                                }
                                if (millis() - presslength > 2000) { // reset stopwatch
                                        iot.pub_msg("Mode: [StopWatch], Counter: [Reset]");
                                        start_stopwatch();
                                }
                                else {
                                        iot.pub_msg("Mode: [StopWatch], Counter: [Continue]");
                                }
                        }
                        iot.pub_msg("Mode: [StopWatch]");
                }
                else {
                        iot.pub_msg("Mode: [Clock]");
                }
                delay(300);
        }

        if (systemState == true) {
                show_stopwatch();
        }
        else {
                show_clock();
        }

        iot.looper();
        delay(500);
}




// //    ~~~~~~~~~ Display clock ~~~~~~~~~~~~~~~~~~~
// Serial.print("Current Time:");
// Serial.print(year(currentBootTime));
// Serial.print("-");
// Serial.print(month(currentBootTime));
// Serial.print("-");
// Serial.print(day(currentBootTime));
// Serial.print(" ");
//
// Serial.print(hour(currentBootTime));
// Serial.print(":");
// Serial.print(minute(currentBootTime));
// Serial.print(":");
// Serial.println(second(currentBootTime));
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Serial.print("Saved Time:");
// Serial.print(year(atoi(savedBoot)));
// Serial.print("-");
// Serial.print(month(atoi(savedBoot)));
// Serial.print("-");
// Serial.print(day(atoi(savedBoot)));
// Serial.print(" ");
//
// Serial.print(hour(atoi(savedBoot)));
// Serial.print(":");
// Serial.print(minute(atoi(savedBoot)));
// Serial.print(":");
// Serial.println(second(atoi(savedBoot)));
// //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Serial.print("Time delta: ");
// Serial.println(currentBootTime - atoi(savedBoot));
