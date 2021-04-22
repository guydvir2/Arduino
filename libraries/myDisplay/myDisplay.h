#ifndef myDisplay_h
#define myDisplay_h
// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class myOLED
{
public:
  int char_size = 2;

private:
#define OLED_RESET LED_BUILTIN

public:
  Adafruit_SSD1306 display;

  myOLED(int height = 64, int width = 128);
  void start();
  void CenterTXT(char *line1 = "", char *line2 = "", char *line3 = "", char *line4 = "", byte x_shift = 0, byte y_shift = 0);
  void RightTXT(char *line1 = "", char *line2 = "", char *line3 = "", char *line4 = "");
  void freeTXT(char *line1 = "", char *line2 = "", char *line3 = "", char *line4 = "");
  void swaper(int swapTime = 5000);
};

class myLCD
{
public:
  myLCD(int lcdColumns = 20, int lcdRows = 4, int lcd_adress = 0x27);
  void start();
  void CenterTXT(char *line1 = "", char *line2 = "", char *line3 = "", char *line4 = "");
  void freeTXT(char *line1 = "", char *line2 = "", char *line3 = "", char *line4 = "");
  void clear();

  LiquidCrystal_I2C lcd;

private:
  int _lcdcols = 0;
};
#endif
