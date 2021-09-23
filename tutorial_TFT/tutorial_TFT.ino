#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 3
#define TS_MIN_X 350
#define TS_MIN_Y 350
#define TS_MAX_X 3800
#define TS_MAX_Y 3800

int TFT_W = 0;
int TFT_H = 0;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS);

void clearScreen()
{
  tft.fillScreen(ILI9341_BLACK);
}
void create_button(uint8_t xc, uint8_t yc, uint8_t a, uint8_t b, char *txt, uint16_t txt_col = ILI9341_BLACK, uint16_t c1 = ILI9341_GREEN, uint16_t c2 = ILI9341_RED, uint8_t txt_size = 1, uint8_t rect_thick = 2)
{
  uint8_t correct_factor = 3;
  tft.setCursor(xc - correct_factor - a / 2, yc - correct_factor);
  tft.setTextColor(txt_col);
  tft.setTextSize(txt_size);
  tft.fillRect(xc - a / 2, yc - b / 2, a, b, c1);

  for (uint8_t x = 0; x < rect_thick * 2; x++)
  {
    tft.drawRect((xc - a / 2) + x / 2, (yc - b / 2) + x / 2, a - x, b - x, c2);
  }
  tft.println(txt);
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

void pressLooper()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    // clearScreen();
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
const uint8_t but_h = 100;
const uint8_t but_w = 50;

const uint8_t cord_but_1[] = {50, 100};
const uint8_t cord_but_2[] = {250, 100};

void display_buttons()
{
  create_button(cord_but_1[0], cord_but_1[1], but_h, but_w, "     BUTTON_1", ILI9341_BLACK, ILI9341_CYAN, ILI9341_GREEN);
  create_button(cord_but_2[0], cord_but_2[1], but_h, but_w, "     BUTTON_2", ILI9341_RED, ILI9341_CYAN);
}
void setup()
{
  Serial.begin(115200);
  ts.begin();
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */

  TFT_W = tft.width();
  TFT_H = tft.height();

  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Rotation:");
  tft.println(SCREEN_ROT * 90);
  tft.println("Size size (WXH):");
  tft.print(TFT_W);
  tft.print("X");
  tft.println(TFT_H);

  // display_buttons();
}

void loop()
{
  pressLooper();
  // if (ts.touched())
  // {

  //   TS_Point p = ts.getPoint();
  //   int x = map(p.x, 350, 3800, 0, 320);
  //   int y = map(p.y, 3800, 400, 0, 240);

  //   if (x <= cord_but_1[0] + 0.5 * but_w && x >= cord_but_1[0] - 0.5 * but_w)
  //   {
  //     if (y <= cord_but_1[1] + 0.5 * but_h && y >= cord_but_1[1] - 0.5 * but_h){
  //       clearScreen();
  //       tft.setCursor(0, 0);
  //       tft.setTextColor(ILI9341_WHITE);
  //       tft.setTextSize(1);
  //       tft.println("Button_1");
  //       delay(200);
  //       display_buttons();
  //     }
  //   }
  //   else if (x <= cord_but_2[0] + 0.5 * but_w && x <= cord_but_2[0] - 0.5 * but_w)
  //   {
  //     if (y <= cord_but_2[1] + 0.5 * but_h && y <= cord_but_2[1] - 0.5 * but_h)
  //     {
  //       clearScreen();
  //       tft.setCursor(0, 0);
  //       tft.setTextColor(ILI9341_WHITE);
  //       tft.setTextSize(1);
  //       tft.println("Button_2");
  //       delay(200);
  //       display_buttons();
  //     }
  //   }
  // }
}
/* 17 char of size 1 @ 100 pixels ;6 line @ 50 chars*/

/* 8 char of size 2 @ 100 pixels*/
