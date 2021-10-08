#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 3
ButtonTFT calib(ts, tft);
int max_x = 0;
int max_y = 0;
int min_x = 9999;
int min_y = 9999;
uint8_t active_button = 0;

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


void clearScreen()
{
  tft.fillScreen(ILI9341_YELLOW);
}
void start_GUI()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  clearScreen();
}
void create_button(char *txt, int posx, int posy, int sizebut_x = 20, int sizebut_y = 0)
{
  if (sizebut_y == 0)
  {
    sizebut_y = sizebut_x;
  }
  calib.screen_rotation = SCREEN_ROT;
  calib.text(txt);
  calib.txt_size = 1;
  calib.a = sizebut_x;
  calib.b = sizebut_y;
  calib.xc = posx;
  calib.yc = posy;
  calib.roundRect = true;
  calib.latchButton = false;
  calib.drawButton();
}
void start_msg(char *msg, int posx, int posy, uint8_t s = 1)
{

  tft.setCursor(posx, posy);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(s);
  tft.print(msg);
}

void update_screen_values(TS_Point &p)
{
  max_x = max((int)p.x, max_x);
  max_y = max((int)p.y, max_y);
  min_x = min((int)p.x, min_x);
  min_y = min((int)p.y, min_y);
}
void print_press_value(TS_Point &p)
{
  Serial.print(p.x);
  Serial.print(",");
  Serial.println(p.y);
  Serial.print("x_min:");
  Serial.print(min_x);
  Serial.print("; x_max:");
  Serial.println(max_x);
  Serial.print("y_min:");
  Serial.print(min_y);
  Serial.print("; y_max:");
  Serial.println(max_y);
}
void topL()
{
  int b = 20;
  active_button = 1;
  create_button("1", b / 2, b / 2, b);
}
void topR()
{
  int b = 20;
  active_button = 2;
  create_button("2", tft.width() - b / 2, b / 2, b);
}
void botR()
{
  int b = 20;
  active_button = 3;
  create_button("3", tft.width() - b / 2, tft.height() - b / 2, b);
}
void botL()
{
  int b = 20;
  active_button = 4;
  create_button("4", b / 2, tft.height() - b / 2, b);
}
void center()
{
  int b = 20;
  active_button = 5;
  create_button("X", tft.width() / 2, tft.height() / 2, b);
}
void restart(){
  active_button = 6;
  create_button("Press Start to Restart", tft.width() / 2, tft.height() / 2, 100, 40); // topL();
}
void setup()
{
  Serial.begin(115200);
  start_GUI();
  active_button = 0;
  create_button("Press Start to Calibrate", tft.width() / 2, tft.height() / 2, 200, 40); // topL();
}

void loop()
{
  static unsigned long last_press = 0;
  if (ts.touched() && millis() - last_press > 500)
  {
    TS_Point p = ts.getPoint();
    if (active_button == 0)
    {
      if (calib.checkPress(p))
      {
        update_screen_values(p);
        clearScreen();
        topL();
      }
    }
    else if (active_button == 1)
    {
      if (calib.checkPress(p))
      {
        update_screen_values(p);
        clearScreen();
        topR();
      }
    }
    else if (active_button == 2)
    {
      if (calib.checkPress(p))
      {
        update_screen_values(p);
        clearScreen();
        botR();
      }
    }
    else if (active_button == 3)
    {
      if (calib.checkPress(p))
      {
        update_screen_values(p);
        clearScreen();
        botL();
      }
    }
    else if (active_button == 4)
    {
      if (calib.checkPress(p))
      {
        update_screen_values(p);
        clearScreen();
        center();
      }
    }
    else if (active_button == 5)
    {
      if (calib.checkPress(p))
      {
        char msg[20];
        clearScreen();
        tft.setCursor(5, 5);
        sprintf(msg, "X{%d:%d}", min_x, max_x);
        tft.print(msg);
        sprintf(msg, "  Y{%d:%d}", min_y, max_y);
        tft.print(msg);
        restart();
      }
    }
    else if (active_button == 6)
    {
      if (calib.checkPress(p))
      {
        update_screen_values(p);
        clearScreen();
        topR();
      }
    }
    last_press = millis();
  }
}
