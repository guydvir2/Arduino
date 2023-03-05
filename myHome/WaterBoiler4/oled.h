#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 // double screen size
#define OLED_RESET LED_BUILTIN

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long clock_noref = 0;

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
    const int timeQoute = 2000;
    const uint8_t screen_lines = 4;

    char timeStamp[15];
    char dateStamp[15];
    static long swapLines_counter = 0;

    time_t t = iot.now();
    struct tm *tm = localtime(&t);
    sprintf(timeStamp, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    sprintf(dateStamp, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

    if (millis() - swapLines_counter > (2 * screen_lines - 2) * timeQoute)
    {
        swapLines_counter = millis();
    }

    for (uint8_t n = 0; n < 2 * screen_lines - 1; n++)
    {
        if (millis() - swapLines_counter > n * timeQoute && millis() - swapLines_counter < (n + 1) * timeQoute)
        {
            char *_lines[screen_lines] = {nullptr, nullptr, nullptr, nullptr};
            if (n < screen_lines)
            {
                _lines[n] = timeStamp;
                _lines[screen_lines - 1 - n] = dateStamp;
            }
            else
            {
                _lines[2 * screen_lines - n - 2] = timeStamp;
                _lines[n - screen_lines + 1] = dateStamp;
            }
            OLED_CenterTXT(2, _lines[0], _lines[1], _lines[2], _lines[3]);
            return;
        }
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
        if (clock_noref != 0 && millis() - clock_noref < 5 * 1000)
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