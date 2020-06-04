#include "Arduino.h"
#include "myDisplay.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


myOLED::myOLED(int height, int width)
    : display(width, height, &Wire, OLED_RESET)
{
}
void myOLED::start()
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
}
void myOLED::CenterTXT(char *line1, char *line2, char *line3, char *line4, byte x_shift, byte y_shift)
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
void myOLED::RightTXT(char *line1, char *line2, char *line3, char *line4)
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
void myOLED::freeTXT(char *line1, char *line2, char *line3, char *line4)
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
            display.setCursor(0, line_space * n);
            display.print(Lines[n]);
        }
    }
    display.display();
}
void myOLED::swaper(int swapTime)
{
    static unsigned long swapLines_counter = 0;
    // char DEGREE_SYMBOL = {167};

    if (swapLines_counter == 0)
    {
        swapLines_counter = millis();
    }
    if (millis() - swapLines_counter < swapTime)
    {
        // CenterTXT(2, timeStamp, dateStamp);
    }
    else if (millis() - swapLines_counter >= swapTime && millis() - swapLines_counter <= 2 * swapTime)
    {
        // OLED_CenterTXT(2, timeStamp, DHTreading);
    }
    else if (millis() - swapLines_counter > 2 * swapTime)
    {
        swapLines_counter = 0;
    }
}

myLCD::myLCD(int lcdColumns, int lcdRows, int lcd_adress)
    : lcd(lcd_adress, lcdColumns, lcdRows)
{
    _lcdcols=lcdColumns;

}

void myLCD::start()
{
    lcd.init();
    lcd.backlight();
}
void myLCD::CenterTXT(char *line1, char *line2, char *line3, char *line4)
{
    char *Lines[] = {line1, line2, line3, line4};
    for (int n = 0; n < 3; n++)
    {
        if (strcmp(Lines[n], "") != 0)
        {
            int strLength = strlen(Lines[n]);
            lcd.setCursor((int)((_lcdcols-strLength) / 2), n);
            lcd.print(Lines[n]);
        }
    }
}
void myLCD::freeTXT(char *line1, char *line2, char *line3, char *line4)
{
    char *Lines[] = {line1, line2, line3, line4};
    for (int n = 0; n < 3; n++)
    {
        if (strcmp(Lines[n], "") != 0)
        {
            lcd.setCursor(0, n);
            lcd.print(Lines[n]);
        }
    }
}