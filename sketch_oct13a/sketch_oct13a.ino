#include <TFT_GUI.h>
#define SCREEN_ROT 0

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

buttonArray_TFT<6> butarr;
buttonArray_TFT<12> keypad;
keypadTFT kpad;

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
void create_keypad()
{
  char *a[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
  // keypad.create_array(4, 3, a);
  kpad.create_keypad();
}
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  start_touchScreen();
  create_keypad();
  // char *a [6] = {"F1", "F2", "F3", "F4", "F5", "F6"};
  // butarr.create_array(3, 2, a);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    keypad.checkPress(p);
  }
}
