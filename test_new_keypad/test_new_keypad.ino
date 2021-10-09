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
keypadTFT keypad(ts, tft);
MessageTFT msgBox(tft);

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
void msg_box(char *msg, uint8_t a, uint8_t b, uint8_t txtsize = 2)
{
  msgBox.a = a;
  msgBox.b = b;
  msgBox.xc = tft.width() / 2;
  msgBox.yc = tft.height() / 2;
  msgBox.txt_size = txtsize;
  msgBox.screen_rotation = SCREEN_ROT;
  msgBox.border_thickness = 1;
  msgBox.face_color = ILI9341_GREEN;
  msgBox.border_color = ILI9341_RED;
  msgBox.txt_color = ILI9341_BLACK;
  bool roundRect = false;
  msgBox.text(msg);
  msgBox.drawMSG();
}
void setup()
{
  Serial.begin(115200);
  start_touchScreen();
  keypad.create_keypad();
}
void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    if (keypad.when_pressed(p)) /* Returns true only when code is ended by "#" */
    {
      clearScreen(2);
      msg_box(keypad.keypad_value, 200, 40);
      delay(2000);
      clearScreen();
      keypad.create_keypad();
    }
  }
}
