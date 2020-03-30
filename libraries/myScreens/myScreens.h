#ifndef myScreens_h
#define myScreens_h
#include <Arduino.h>

//~~~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ~~~~~~~~~~~~~~~~~ USE USE_LCD ~~~~~~~~~~~~
#include <LiquidCrystal_I2C.h>

class myScreens
{
    #define OLED_RESET LED_BUILTIN
private:
int _lcdColumns = 16;
int _lcdRows = 2;
public:
    myScreens(char *screen_type = "lcd", int rows = 2, int cols = 16);
    void startLCD();
};
#endif
