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

MessageTFT msgBox(tft);
ButtonTFT button(ts, tft);

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


void setup()
{
  Serial.begin(115200);
  start_touchScreen();
  msgBox.screen_rotation = SCREEN_ROT;
  msgBox.drawMSG("GUY_DVIR", 100, 50, tft.width() / 2, tft.height() / 2);

  button.screen_rotation = SCREEN_ROT;
  button.drawButton("Butt-Me", 100, 50, tft.width() / 2, 25);
}

void loop()
{
  static unsigned long lastPress = 0;
  if (ts.touched() && millis() - lastPress > 500)
  {
    TS_Point p = ts.getPoint();
    lastPress = millis();
    if (button.checkPress(p))
    {
      Serial.println("Ress");
    }
  }
}
