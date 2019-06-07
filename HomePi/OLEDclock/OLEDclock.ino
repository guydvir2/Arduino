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
#define DEVICE_TOPIC        "HomePi/Dvir/OLED_Clock"
#define ADD_MQTT_FUNC       addiotnalMQTT
#define USE_SERIAL          false
#define USE_WDT             true
#define USE_OTA             true
#define USE_FAT             true // Flash Assist

#define VER                 "Wemos_V2.0"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~ myJSON  ~~~~~~~~~~~~~~~~~~~~~
#define jfile "myfile.json"
#define DISPMODE_KEY        "saved_dispmode"
#define BOOT_CALC_KEY       "BootTime1"
#define BOOT_RESET_KEY      "BootTime2"
#define STPWATCH_KEY        "savedStopWatch"

// Read from FLASH
long savedBoot_Calc     = 0; // boot time for calc
long savedBoot_reset    = 0; // last actual boot
long savedStopwatch     = 0; // clock when stopwatch start
int savedDispMode       = 0;

int allowedTime_betweenReboots = 30; //secץ time between reboots to be considered as continuation
// int allowedTime_betweenReboots = 99999; // considered as inf time
// int allowedTime_betweenReboots = 0; // considered as every reboot an new operation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~ Hardware setup   ~~~~~~~~~~~
#define buttonPin       D4
#define PRESSED         LOW
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ StopWatch ~~~~~~~~~~~~~~~~~~~~
byte days       = 0;
byte hours      = 0;
byte minutes    = 0;
byte seconds    = 0;

int sec2minutes = 60;
int sec2hours   = (sec2minutes * 60);
int sec2days    = (sec2hours * 24);
int sec2years   = (sec2days * 365);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ System States  ~~~~~~~~~~~~~~~
bool disp_stopWatch_flag  = false; // display clock or stopwatch
bool resetBoot_flag       = false; // detect boot due to reset
bool stopWatch_ON         = false; // Stopwatch is running
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

long stopWatch_clocklStart   = 0;
long updated_bootTime        = 0;

// ~~~~~~~~~~~~~~Start services ~~~~~~~~~~~~
myIOT iot(DEVICE_TOPIC);
myJSON json(jfile, true);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~








// ~~~~~~~~~~~~~~ display OLED ~~~~~~~~~~~~~
void center_text(int i, char *line1, char *line2 = "", char *line3 = "",
                 char *line4 = "", byte shift = 0) {
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
        /*  size: 1 = 21X4
           /   size: 2 = 10X2
            size: 4 = 5 X1
         */
}
void displayClock_OLED() {
        char clock[12];
        char date[12];
        iot.return_clock(clock);
        iot.return_date(date);
        center_text(2, clock, date);
}
void displayStopWatch_OLED() {
        char day_char[12];
        char clock_char[50];

        convert_epoch2clock(now(), stopWatch_clocklStart, day_char, clock_char);
        sprintf(day_char, "%02d days", days);
        sprintf(clock_char, "%02d:%02d:%02d", hours, minutes, seconds);
        center_text(2, day_char, clock_char);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ Create Clocks ~~~~~~~~~~~
void convert_epoch2clock(long t1, long t2, char* time_str, char* date_str){
        long time_delta = t1-t2;

        days    = (int)(time_delta / sec2days);
        hours   = (int)((time_delta - days * sec2days) / sec2hours);
        minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
        seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

        sprintf(time_str, "%02d days", days);
        sprintf(date_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
void getTime_stamp(time_t t, char *ret_date, char* ret_clock){
        sprintf(ret_date, "%04d-%02d-%02d", year(t), month(t), day(t));
        sprintf(ret_clock, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ StopWatch ~~~~~~~~~~~~~~~~~~
void start_stopwatch(long defVal = updated_bootTime) {
        stopWatch_clocklStart = defVal;
        stopWatch_ON = true;
        disp_stopWatch_flag = true; // stopwatch mode display
        if (USE_FAT) {
                json.setValue(DISPMODE_KEY, 1);
                json.setValue(STPWATCH_KEY, stopWatch_clocklStart);
        }

}
void stop_stopwatch() {
        stopWatch_ON = false;
        disp_stopWatch_flag = !true; // back to clock mode display
        if (USE_FAT) {
                json.setValue(DISPMODE_KEY, 0);
                json.setValue(STPWATCH_KEY, 0);
        }

}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Load Saved Flash ~~~~~~~~~~~
bool load_bootTime() {
        if (json.getValue(BOOT_CALC_KEY, savedBoot_Calc)) {
                // Serial.print("Boot retrieved: ");
                // Serial.println(savedBoot_Calc);
        }
        else {
                json.setValue(BOOT_CALC_KEY, 0);
                // Serial.println("Boot default saved");
        }
        if (json.getValue(BOOT_RESET_KEY, savedBoot_reset)) {
                // Serial.print("Boot retrieved: ");
                // Serial.println(savedBoot_reset);
        }
        else {
                json.setValue(BOOT_CALC_KEY, 0);
                // Serial.println("Boot default saved");
        }

        long currentBootTime = now();
        int x =0;

        while (x<5) { // verify time is updated
                if (year(currentBootTime) != 1970) { //NTP update succeeded
                        json.setValue(BOOT_RESET_KEY, currentBootTime);
                        break;
                }
                else{
                        currentBootTime = now();
                }
                x +=1;
                delay(200);
        }
        if (x==4) { // fail NTP
                return 0;
        }

        if (currentBootTime - savedBoot_reset > allowedTime_betweenReboots ) {
                // Serial.print("TimeDelta:");
                // Serial.println(currentBootTime - savedBoot_reset);
                json.setValue(BOOT_CALC_KEY, currentBootTime);
                // Serial.println("Time updated");
                updated_bootTime = currentBootTime;                   // take clock of current boot
                return 1;
        }
        else if ( currentBootTime - savedBoot_reset <= allowedTime_betweenReboots) {
                // Serial.println("No time update");
                updated_bootTime = savedBoot_Calc;                   // take clock of last boot
                resetBoot_flag = true;
                return 1;
        }
}
void load_dispMode() {
        if (json.getValue(DISPMODE_KEY, savedDispMode)) {
                // Serial.print("SystemState retrieved: ");
                // Serial.println(savedDispMode);

                if (resetBoot_flag==0) {
                        disp_stopWatch_flag = false;
                }
                else{
                        disp_stopWatch_flag = savedDispMode;
                }

        }
        else {
                json.setValue(DISPMODE_KEY,0);
                // Serial.println("State default saved");
                disp_stopWatch_flag = false; // show watch
        }
}
void load_stopwatchValue() {
        if (json.getValue(STPWATCH_KEY, savedStopwatch)) {
                if (savedStopwatch > 0 && resetBoot_flag == true) {
                        start_stopwatch(updated_bootTime);

                        // Serial.print("Stopwatch retrieved: ");
                        // Serial.println(savedStopwatch);
                }
                else if(resetBoot_flag == false) {
                        disp_stopWatch_flag = false;
                }
        }
        else {
                json.setValue(STPWATCH_KEY, 0);
                // Serial.println("StopWatch default saved");
                disp_stopWatch_flag = false;
        }
}
void load_allFLASH_vars(){

  if (load_bootTime()==true){
    load_dispMode();
    load_stopwatchValue();
  }
  else{
    center_text(2,"Clock Update","Error");

  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void readButton(){
        if (digitalRead(buttonPin) == PRESSED ) {
                delay(50); // debounce
                if (digitalRead(buttonPin) == PRESSED) {
                        unsigned long presslength = millis();
                        disp_stopWatch_flag = !disp_stopWatch_flag; // change state
                        if (disp_stopWatch_flag == true) { //true for stopwatch mode
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
}
void addiotnalMQTT(char *incoming_msg) {
        char msg[100];

        if (strcmp(incoming_msg, "status") == 0) {
                sprintf(msg, "Status: [%s] Mode", disp_stopWatch_flag ? "StopWatch" : "Clock");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "start") == 0) {
                sprintf(msg, "StopWatch: [start]");
                start_stopwatch(now());
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "stop") == 0 ) {
                sprintf(msg, "StopWatch: [stop]");
                stop_stopwatch();
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "clock") == 0) {
                sprintf(msg, "Mode: [Clock]");
                disp_stopWatch_flag = !true;
                json.setValue(DISPMODE_KEY, 0);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "stopwatch") == 0) {
                sprintf(msg, "Mode: [StopWatch]");
                disp_stopWatch_flag = true;
                json.setValue(DISPMODE_KEY, 1);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: [status, start, stop, clock, stopwatch, uptime, format, help] , [ver, boot, reset, ip, ota]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], FAT:[%d], tBetweenReset[%d sec]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_FAT, allowedTime_betweenReboots);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "uptime") == 0 ) {
                sprintf(msg, "UpTime: %02d days  %02d:%02d:%02d", days, hours, minutes, seconds);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                sprintf(msg, "Flash Format started");
                json.format();
                iot.pub_msg(msg);
        }
}

void setup() {
        pinMode(buttonPin, INPUT_PULLUP);

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);

        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

        if (USE_FAT) { //Flash AssisT
                load_allFLASH_vars();
        }
        else{
                updated_bootTime = now();
        }
}

void loop() {
        readButton();

        if (disp_stopWatch_flag == true && stopWatch_ON == true ) {
                displayStopWatch_OLED();
        }
        else {
                displayClock_OLED();
        }

        iot.looper();
        delay(300);
}
