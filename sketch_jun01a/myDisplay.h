#ifndef myDisplay_h
#define myDisplay_h

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN
#define SCREEN_WIDTH 128

#if displayRows == 4
#define SCREEN_HEIGHT 64 // 32 2rows or 64 4 rows
#else if displayRows == 2
#define SCREEN_HEIGHT 32 // 32 2rows or 64 4 rows
#endif

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


class myDisplay {
  private:
  public:

};
