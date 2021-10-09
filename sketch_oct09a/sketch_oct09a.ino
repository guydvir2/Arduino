#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 0

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

buttonArrayTFT butArray(ts, tft);

void clearScreen(int c = 0)
{
  if (c == 0)
  {
    tft.fillScreen(ILI9341_BLACK);
  }
  else if (c == 1)
  {
    tft.fillScreen(ILI9341_YELLOW);
  }
  else if (c == 2)
  {
    tft.fillScreen(ILI9341_BLUE);
  }
}
void start_touchScreen()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  clearScreen();
}
void build_butArray()
{
  clearScreen();
  char *txt_buttons[] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"};
  butArray.screen_rotation = SCREEN_ROT;
  butArray.create_array(3, 2, txt_buttons, 2);
}

void setup()
{
  Serial.begin(115200);
  start_touchScreen();
  build_butArray();
}

void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    // butArray.checkPress(p);
    Serial.print("Button #");
    Serial.println(butArray.checkPress(p));
  }
}
