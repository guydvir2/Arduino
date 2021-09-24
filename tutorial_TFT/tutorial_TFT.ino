#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
// #include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 0
#define TS_MIN_X 350
#define TS_MIN_Y 350
#define TS_MAX_X 3800
#define TS_MAX_Y 3800


XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

int TFT_W = 0;
int TFT_H = 0;

void start_GUI()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  TFT_W = tft.width();
  TFT_H = tft.height();
  tft.fillScreen(ILI9341_BLUE);
}
void GUI_properties()
{
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Rotation:");
  tft.print(SCREEN_ROT * 90);
  tft.print("Size  (WXH):");
  tft.print(TFT_W);
  tft.print("X");
  tft.print(TFT_H);
}

void setup()
{
  Serial.begin(115200);
  start_GUI();
  GUI_properties();
}

void loop()
{
  draw_screen();
}

void clearScreen()
{
  tft.fillScreen(ILI9341_BLACK);
}
int TS2TFT_x(int px)
{
  if (SCREEN_ROT == 0)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, TFT_H);
  }
  else if (SCREEN_ROT == 1)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, TFT_W);
  }
  else if (SCREEN_ROT == 2)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, TFT_H);
  }
  else if (SCREEN_ROT == 3)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, TFT_W);
  }
}
int TS2TFT_y(int py)
{

  if (SCREEN_ROT == 0)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, TFT_W);
  }
  else if (SCREEN_ROT == 1)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, TFT_H);
  }
  else if (SCREEN_ROT == 2)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, TFT_W);
  }
  else if (SCREEN_ROT == 3)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, TFT_H);
  }
}
void draw_screen()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    int x, y;
    if (SCREEN_ROT == 1 || SCREEN_ROT == 3)
    {
      x = TS2TFT_x(p.x);
      y = TS2TFT_y(p.y);
    }
    else
    {
      y = TS2TFT_x(p.x);
      x = TS2TFT_y(p.y);
    }
    tft.setCursor(0, 0);
    tft.print(p.x);
    tft.print(",");
    tft.print(p.y);

    tft.setCursor(TFT_W / 2, TFT_H / 2);
    tft.print(x);
    tft.print(",");
    tft.print(y);

    tft.setCursor(x, y);
    tft.print("*");
    // const int rsize = 150;
    // tft.fillRect(x - rsize / 2, y - rsize / 2, rsize, rsize, ILI9341_GREEN);

    //    tft.print("Pressure = ");
    //    tft.println(p.z);
    //    tft.print("X = ");
    //    tft.println(p.x);
    //    tft.print("Y = ");
    //    tft.println(p.y);
  }
}
