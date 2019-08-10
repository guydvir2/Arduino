#include <myIOT.h>
#include <Arduino.h>

//~~~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       32    // 32 2rows or 64 4 rows
#define OLED_RESET          LED_BUILTIN
#define buttonPin           D4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~ myIOT  ~~~~~~~~~~~~~~~~~~~~~
#define DEVICE_TOPIC        "Clock"
#define MQTT_PREFIX         "myHome"
#define MQTT_GROUP          "OLED"
#define ADD_MQTT_FUNC       addiotnalMQTT
#define USE_SERIAL          true
#define USE_WDT             true
#define USE_OTA             true
#define USE_FAT             true // Flash Assist
#define USE_RESETKEEPER     false
#define USE_FAILNTP         true

#define VER                 "Wemos_V3.0"
myIOT iot(DEVICE_TOPIC);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

void addiotnalMQTT(char *incoming_msg) {
        // char msg[100];
        //
        // if (strcmp(incoming_msg, "status") == 0) {
        //         sprintf(msg, "Status: [%s] Mode", disp_stopWatch_flag ? "StopWatch" : "Clock");
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "start") == 0) {
        //         sprintf(msg, "StopWatch: [start]");
        //         start_stopwatch(now());
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "stop") == 0 ) {
        //         sprintf(msg, "StopWatch: [stop]");
        //         stop_stopwatch();
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "clock") == 0) {
        //         sprintf(msg, "Mode: [Clock]");
        //         disp_stopWatch_flag = !true;
        //         json.setValue(DISPMODE_KEY, 0);
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "stopwatch") == 0) {
        //         sprintf(msg, "Mode: [StopWatch]");
        //         disp_stopWatch_flag = true;
        //         json.setValue(DISPMODE_KEY, 1);
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "help") == 0) {
        //         sprintf(msg, "Help: [status, start, stop, clock, stopwatch, uptime, format, help] , [ver, boot, reset, ip, ota]");
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "ver") == 0 ) {
        //         sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d], FAT:[%d], tBetweenReset[%d sec]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_FAT, allowedTime_betweenReboots);
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "uptime") == 0 ) {
        //         sprintf(msg, "UpTime: %02d days  %02d:%02d:%02d", days, hours, minutes, seconds);
        //         iot.pub_msg(msg);
        // }
        // else if (strcmp(incoming_msg, "format") == 0 ) {
        //         sprintf(msg, "Flash Format started");
        //         json.format();
        //         iot.pub_msg(msg);
        // }
}

void setup() {
        pinMode(buttonPin, INPUT_PULLUP);

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);

        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}
void loop() {
        displayClock_OLED();

        iot.looper();
        delay(300);
}
