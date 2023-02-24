#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "constants.h"
#include <myIOT2.h>
#include "myIOT_settings.h"
#include "TO_settings.h"

unsigned long clock_noref = 0;
unsigned long onclk = 0;

// ~~~~~~~~~~~~~~ OLED display ~~~~~~~~~~~~~~~~~
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void startOLED()
{
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
}
uint8_t prep_display(uint8_t s = 2, uint8_t c = 0)
{
        display.clearDisplay();
        display.setTextSize(s);
        if (c == 0)
        {
                display.setTextColor(WHITE);
        }
        else if (c == 1)
        {
                display.setTextColor(WHITE);
        }
        return pow(2, (2 + s));
}
uint8_t x_cursor(uint8_t s, uint8_t num_charac)
{
        float char_size_in_pixels = (float)128 / ((float)21 / (float)s);
        float place_col_in_chars = (float)21 / (float)s - num_charac;
        return (ceil)(place_col_in_chars * char_size_in_pixels);
}
void OLED_CenterTXT(uint8_t char_size, const char *line1, const char *line2 = nullptr, const char *line3 = nullptr, const char *line4 = nullptr)
{
        const char *Lines[] = {line1, line2, line3, line4};
        byte line_space = prep_display(char_size, 0);

        for (uint8_t n = 0; n < 4; n++)
        {
                if (Lines[n] != nullptr)
                {
                        // display.setCursor((ceil)((21 / char_size - strlen(Lines[n])) / 2 * (128 / (21 / char_size))), line_space * n);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void OLED_SideTXT(uint8_t char_size, const char *line1, const char *line2 = nullptr, const char *line3 = nullptr, const char *line4 = nullptr)
{
        const char *Lines[] = {line1, line2, line3, line4};
        byte line_space = prep_display(char_size, 0);

        if (line3 == nullptr && line4 == nullptr)
        { // for ON state only - 2rows
                for (uint8_t n = 0; n < 2; n++)
                {
                        if (Lines[n] != nullptr)
                        {
                                if (n == 1)
                                { // Clock line
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        // display.setCursor((ceil)((21 / char_size - strlen(Lines[n]) * (128 / (21 / char_size))), line_space * (n + 1) - 3);
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
                        if (Lines[n] != nullptr)
                        {
                                if (n == 1 || n == 3)
                                { // Clocks
                                        display.setTextSize(char_size);
                                        display.setTextColor(WHITE);
                                        int strLength = strlen(Lines[n]);
                                        // display.setCursor((ceil)((21 / char_size - strLength) * (128 / (21 / char_size))), line_space * n - 3);
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
void display_totalOnTime()
{
        char msg[50];
        iot.convert_epoch2clock(iot.now(), onclk, msg);
        // sec2clock((int)(iot.now() - onclk), msg);
        clock_noref = millis(); // start clock to Frozen msg
        OLED_CenterTXT(2, "Total", "ON time:", msg);
}
void display_ON_clock()
{
        char time_on_char[20];
        char time2Off_char[20];

        if (onclk == 0)
        {
                onclk = TOswitch.onClk();
        }

        // sec2clock((int)() onclk), time_on_char);
        iot.convert_epoch2clock(iot.now(), onclk, time_on_char);
        if ((int)(iot.now() - onclk) == 0)
        {
                OLED_SideTXT(2, "On:", time_on_char);
        }
        else
        {
                // sec2clock(TOswitch.remTime(), time2Off_char);
                // iot.convert_epoch2clock(iot.now(),onclk,msg);

                OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
        }
}
void display_OFF_clock()
{
        char timeStamp[15];
        char dateStamp[15];
        static long swapLines_counter = 0;

        // iot.return_clock(timeStamp);
        // iot.return_date(dateStamp);

        int timeQoute = 5000;

        if (swapLines_counter == 0)
        {
                swapLines_counter = millis();
        }
        if (millis() - swapLines_counter < timeQoute)
        {
                OLED_CenterTXT(2, nullptr, timeStamp, dateStamp, nullptr);
        }
        else if (millis() - swapLines_counter >= timeQoute && millis() - swapLines_counter < 2 * timeQoute)
        {
                OLED_CenterTXT(2, timeStamp, nullptr, nullptr, dateStamp);
        }
        else if (millis() - swapLines_counter > 2 * timeQoute)
        {
                swapLines_counter = 0;
        }
}
void OLEDlooper()
{
        if (clock_noref == 0) // OLED is not Frozen with a static msg
        {
                if (digitalRead(RELAY1) == RelayOn)
                {
                        display_ON_clock();
                }
                else
                { // OFF state - clock only
                        display_OFF_clock();
                }
        }

        else /* Display Frozen msg*/
        {
                if (millis() - clock_noref > time_NOref_OLED * 1000)
                {                        // time in millis without screen refresh
                        clock_noref = 0; /* stop Frozen display */
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void initGPIO()
{
        pinMode(indic_LEDpin, OUTPUT);
        pinMode(RELAY1, OUTPUT);
}
void setup()
{
        initGPIO();
        startIOTservices();
        // TOswitch_init();
        // startOLED();
}
void loop()
{
        iot.looper();
        // OLEDlooper();
        // TOswitch.looper();
        // delay(100);
}
