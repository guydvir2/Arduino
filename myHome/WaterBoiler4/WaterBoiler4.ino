#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <myIOT2.h>
#include <smartSwitch.h>

myIOT2 iot;
smartSwitch SWitch;

#include "constants.h"
#include "myIOT_settings.h"

unsigned long clock_noref = 0;
const char *REASONS_OPER[] = {"Button", "Timeout", "MQTT"};
void init_smartSwitch()
{
        SWitch.OUTPUT_ON = HIGH;
        SWitch.BUTTON_PRESSED = LOW;

        SWitch.set_id(1);
        SWitch.set_timeout(5);
        SWitch.set_name("boiler");
        SWitch.set_input(INPUT1, MULTI_PRESS_BUTTON);
        SWitch.set_output(RELAY1);
        SWitch.set_indiction(indic_LEDpin, HIGH);
}
void smartSwitch_loop()
{
        if (SWitch.loop())
        {
                char clk[15];
                char msg[100];
                const char *states[] = {"Off", "On"};
                if (SWitch.telemtryMSG.state == 0)
                {
                        iot.convert_epoch2clock(SWitch.get_elapsed() / 1000, 0, clk);
                        clock_noref = millis();
                        sprintf(msg, "[%s]: turned [%s], timeout [%s]", REASONS_OPER[SWitch.telemtryMSG.reason],
                                states[SWitch.telemtryMSG.state], clk);
                }
                else if (SWitch.telemtryMSG.state == 1)
                {
                        if (SWitch.telemtryMSG.pressCount > 1)
                        {
                                const int add_time = 12;
                                iot.convert_epoch2clock(SWitch.get_timeout() / 1000, 0, clk);
                                SWitch.set_additional_timeout(add_time, SWitch.telemtryMSG.reason);
                                sprintf(msg, "[%s]: added [%d] minutes, timeout [%s]", REASONS_OPER[SWitch.telemtryMSG.reason],
                                        add_time, clk);
                        }
                        else
                        {
                                iot.convert_epoch2clock(SWitch.get_timeout() / 1000, 0, clk);
                                sprintf(msg, "[%s]: turned [%s], timeout [%s]", REASONS_OPER[SWitch.telemtryMSG.reason],
                                        states[SWitch.telemtryMSG.state], clk);
                        }
                }
                iot.pub_msg(msg);
                SWitch.clear_newMSG();
        }
}

// ~~~~~~~~~~~~~~ OLED display ~~~~~~~~~~~~~~~~~
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void startOLED()
{
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
}
uint8_t prep_display(uint8_t s = 2, uint8_t c = 0)
{
        display.setTextSize(s);

        if (c == 0)
        {
                display.setTextColor(WHITE);
        }
        else if (c == 1)
        {
                display.setTextColor(WHITE);
        }
        else if (c == 2)
        {
                display.setTextColor(BLACK, WHITE);
        }
        return pow(2, (2 + s));
}
uint16_t x_cursor(uint8_t s, uint8_t num_charac)
{
        float char_size_in_pixels = (float)SCREEN_WIDTH / ((float)21 / (float)s);
        float place_col_in_chars = (float)21 / (float)s - num_charac;
        return (ceil)(char_size_in_pixels * place_col_in_chars);
}
void OLED_CenterTXT(uint8_t char_size, const char *line1, const char *line2 = nullptr, const char *line3 = nullptr, const char *line4 = nullptr)
{
        const char *Lines[] = {line1, line2, line3, line4};
        uint8_t line_space = prep_display(char_size, 0);
        display.clearDisplay();

        for (uint8_t n = 0; n < 4; n++)
        {
                if (Lines[n] != nullptr)
                {
                        display.setCursor(x_cursor(char_size, strlen(Lines[n])) / 2, line_space * n);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void OLED_SideTXT(uint8_t char_size, const char *line1, const char *line2 = nullptr, const char *line3 = nullptr, const char *line4 = nullptr)
{
        uint8_t line_space = 0;
        const char *Lines[] = {line1, line2, line3, line4};
        display.clearDisplay();
        uint8_t x = 0;
        uint8_t y = 0;

        for (uint8_t n = 0; n < 4; n++)
        {
                if (Lines[n] != nullptr)
                {
                        if (n % 2 == 1)
                        {
                                line_space = prep_display(char_size, 0);
                                x = x_cursor(char_size, strlen(Lines[n]));
                                y = line_space * n;
                        }
                        else
                        {
                                line_space = prep_display(char_size - 1, 0);
                                x = 0;
                                y = line_space * 2 * n + 5;
                        }
                        // line_space = prep_display(char_size, 0);
                        display.setCursor(x, y);
                        display.print(Lines[n]);
                }
        }
        display.display();
}
void display_totalOnTime()
{
        char msg[50];
        iot.convert_epoch2clock(SWitch.get_elapsed() / 1000, 0, msg);
        OLED_CenterTXT(2, "Total", "ON time:", msg);
}
void display_on_duration()
{
        char time_on_char[20];
        char time2Off_char[20];
        iot.convert_epoch2clock(SWitch.get_elapsed() / 1000, 0, time_on_char);
        iot.convert_epoch2clock(SWitch.get_remain_time() / 1000 + 1, 0, time2Off_char);
        OLED_SideTXT(2, "On:", time_on_char, "Remain:", time2Off_char);
}
void display_clock()
{
        char timeStamp[15];
        char dateStamp[15];
        static long swapLines_counter = 0;

        time_t t = iot.now();
        struct tm *tm = localtime(&t);
        sprintf(timeStamp, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
        sprintf(dateStamp, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

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
void OLED_display_looper()
{
        if (SWitch.get_SWstate() == 1)
        {
                display_on_duration();
        }
        else
        { // OFF state - clock only
                if (clock_noref != 0 && millis() - clock_noref < time_NOref_OLED * 1000)
                { // time in millis without screen refresh
                        display_totalOnTime();
                }
                else
                {
                        clock_noref = 0;
                        display_clock();
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
        startIOTservices();
        startOLED();
        init_smartSwitch();
}
void loop()
{
        iot.looper();
        OLED_display_looper();
        smartSwitch_loop();
}
