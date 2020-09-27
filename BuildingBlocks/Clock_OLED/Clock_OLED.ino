#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_2.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
#define USE_DHT true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "Clock_DHT"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

//~~~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 // 32 2rows or 64 4 rows
#define OLED_RESET LED_BUILTIN

long swapLines_counter = 0;
char timeStamp[50];
char dateStamp[50];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ DHT ~~~~~~~~~~~
#if USE_DHT
#include "DHT.h"
#define DHTPIN D4
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
float humidity;
float temperature;
long lastRead_DHT = 0;
long readInterval = 10000;
DHT dht(DHTPIN, DHTTYPE);

void startDHT()
{
        dht.begin();
}

void readDHT()
{
        static bool readDHTonce = false;
        if (millis() - lastRead_DHT > readInterval)
        {
                humidity = dht.readHumidity();
                temperature = dht.readTemperature();
                lastRead_DHT = millis();
        }
}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0)
        {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0)
        {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
        }
}

// ~~~~ OLED ~~~~~~~
void startOLED()
{
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
}
void OLED_CenterTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "", byte x_shift = 0, byte y_shift = 0)
{
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        display.setTextColor(WHITE);
        byte line_space = pow(2, (2 + char_size));

        for (int n = 0; n < 4; n++)
        {
                if (strcmp(Lines[n], "") != 0)
                {
                        int strLength = strlen(Lines[n]);
                        display.setCursor((ceil)((21 / char_size - strLength) / 2 * (128 / (21 / char_size))) + x_shift, line_space * n + y_shift);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void OLED_SideTXT(int char_size, char *line1, char *line2 = "", char *line3 = "", char *line4 = "")
{
        char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        display.setTextSize(char_size);
        byte line_space = pow(2, (2 + char_size));

        if (strcmp(line3, "") == 0 && strcmp(line4, "") == 0)
        { // for ON state only - 2rows
                for (int n = 0; n < 2; n++)
                {
                        if (strcmp(Lines[n], "") != 0)
                        {
                                if (n == 1)
                                { // Clock line
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * (n + 1) - 3);
                                        display.print(Lines[n]);
                                }
                                else
                                { // Title line
                                        display.setTextSize(char_size - 1);
                                        display.setTextColor(BLACK, WHITE);
                                        display.setCursor(0, line_space * (n + 1));
                                        display.print(Lines[n]);
                                }
                        }
                }
        }
        else
        {
                for (int n = 0; n < 4; n++)
                {
                        if (strcmp(Lines[n], "") != 0)
                        {
                                if (n == 1 || n == 3)
                                { // Clocks
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * n - 3);
                                        display.print(Lines[n]);
                                }
                                else
                                { // Title
                                        display.setTextSize(char_size - 1);
                                        display.setTextColor(BLACK, WHITE);
                                        display.setCursor(0, line_space * n);
                                        display.print(Lines[n]);
                                }
                        }
                }
        }
        display.display();
}
void OLEDlooper(int swapTime = 5000)
{
        iot.return_clock(timeStamp);
        iot.return_date(dateStamp);

#if USE_DHT
        char temp_char[20];
        char h_char[20];
        readDHT();
        sprintf(temp_char,"T: %.1f%cC",temperature,247);
        sprintf(h_char, "Humid: %.0f%%", humidity);

        OLED_CenterTXT(2, timeStamp, dateStamp, temp_char, h_char);
#endif

#if !USE_DHT
        if (swapLines_counter == 0)
        {
                swapLines_counter = millis();
        }
        if (millis() - swapLines_counter < swapTime)
        {
                OLED_CenterTXT(2, "", timeStamp, dateStamp);
        }
        else if (millis() - swapLines_counter >= swapTime && millis() - swapLines_counter < 2 * swapTime)
        {
                OLED_CenterTXT(2, timeStamp, "", "", dateStamp);
        }
        else if (millis() - swapLines_counter > 2 * swapTime)
        {
                swapLines_counter = 0;
        }
#endif
}
void setup()
{
        startIOTservices();
        startOLED();
#if USE_DHT
        startDHT();
#endif
}
void loop()
{
        iot.looper();
        OLEDlooper();
        delay(100);
}
