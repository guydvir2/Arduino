#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 0
#define TS_MIN_X 350
#define TS_MIN_Y 350
#define TS_MAX_X 3800
#define TS_MAX_Y 3800

typedef void (*cb_func)();

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

int TFT_W = 0;
int TFT_H = 0;

class ButtonTFT
{
#define TS_MIN_X 350
#define TS_MIN_Y 350
#define TS_MAX_X 3800
#define TS_MAX_Y 3800
public:
  uint8_t xc, yc;
  uint8_t a, b;
  uint8_t txt_size = 1;
  uint8_t border_thickness = 2;
  uint8_t screen_rotation = 0;

  uint16_t face_color = ILI9341_GREEN;
  uint16_t border_color = ILI9341_RED;
  uint16_t txt_color = ILI9341_BLACK;

  char txt_buf[15];

public:
  ButtonTFT(XPT2046_Touchscreen &ts, Adafruit_ILI9341 &tft)
  {
  }
  void init()
  {
    _TFT_W = tft.width();
    _TFT_H = tft.height();
  }
  void drawButton()
  {
    _construct_button();
    _put_text();
  }
  bool wait4press()
  {
    if (ts.touched())
    {
      if (_isPressed == false)
      {
        _isPressed = true;
        TS_Point p = ts.getPoint();
        _conv_ts_tft(p);
        if (_tft_x <= xc + a / 2 && _tft_x >= xc - a / 2)
        {
          if (_tft_y <= yc + b / 2 && _tft_y >= yc - b / 2)
          {
            _press_cb();
            return 1;
          }
        }
        else
        {
          return 0;
        }
      }
    }
    else
    {
      _isPressed = false;
      return 0;
    }
  }

private:
  int _TFT_W = 0;
  int _TFT_H = 0;
  int _tft_x, _tft_y;
  uint8_t correct_factor[3] = {3, 5, 9};
  bool _isPressed = false;

private:
  void _construct_button()
  {
    tft.setCursor(xc - correct_factor[txt_size - 1] - a / 2, yc - correct_factor[txt_size - 1]);
    tft.fillRect(xc - a / 2, yc - b / 2, a, b, face_color);
    for (uint8_t x = 0; x < border_thickness * 2; x++)
    {
      tft.drawRect((xc - a / 2) + x / 2, (yc - b / 2) + x / 2, a - x, b - x, border_color);
    }
  }
  void _put_text()
  {
    uint8_t x = strlen(txt_buf);
    tft.setCursor(xc - x * correct_factor[txt_size - 1], yc - correct_factor[txt_size - 1]);
    tft.setTextColor(txt_color);
    tft.setTextSize(txt_size);
    tft.print(txt_buf);
  }
  void _press_cb()
  {
    uint16_t _old_fc = face_color;
    uint16_t _old_bc = border_color;
    face_color = border_color;
    border_color = _old_fc;

    drawButton();
    delay(200);
    face_color = _old_fc;
    border_color = _old_bc;
    drawButton();
  }
  void _conv_ts_tft(TS_Point &p)
  {

    if (screen_rotation == 1 || screen_rotation == 3)
    {
      _tft_x = _TS2TFT_x(p.x);
      _tft_y = _TS2TFT_y(p.y);
    }
    else
    {
      _tft_y = _TS2TFT_x(p.x);
      _tft_x = _TS2TFT_y(p.y);
    }
  }
  int _TS2TFT_x(int px)
  {
    if (SCREEN_ROT == 0)
    {
      return map(px, TS_MAX_X, TS_MIN_X, 0, _TFT_H);
    }
    else if (SCREEN_ROT == 1)
    {
      return map(px, TS_MAX_X, TS_MIN_X, 0, _TFT_W);
    }
    else if (SCREEN_ROT == 2)
    {
      return map(px, TS_MIN_X, TS_MAX_X, 0, _TFT_H);
    }
    else if (SCREEN_ROT == 3)
    {
      return map(px, TS_MIN_X, TS_MAX_X, 0, _TFT_W);
    }
  }
  int _TS2TFT_y(int py)
  {

    if (SCREEN_ROT == 0)
    {
      return map(py, TS_MAX_Y, TS_MIN_Y, 0, _TFT_W);
    }
    else if (SCREEN_ROT == 1)
    {
      return map(py, TS_MIN_Y, TS_MAX_Y, 0, _TFT_H);
    }
    else if (SCREEN_ROT == 2)
    {
      return map(py, TS_MIN_Y, TS_MAX_Y, 0, _TFT_W);
    }
    else if (SCREEN_ROT == 3)
    {
      return map(py, TS_MAX_Y, TS_MIN_Y, 0, _TFT_H);
    }
  }
};

ButtonTFT But_1(ts, tft);
ButtonTFT But_2(ts, tft);
ButtonTFT But_3(ts, tft);
ButtonTFT But_4(ts, tft);
ButtonTFT But_5(ts, tft);
ButtonTFT But_6(ts, tft);
ButtonTFT But_7(ts, tft);
ButtonTFT But_8(ts, tft);
ButtonTFT But_9(ts, tft);
ButtonTFT But_10(ts, tft);
ButtonTFT But_11(ts, tft);
ButtonTFT But_12(ts, tft);

ButtonTFT *buttons[] = {&But_1, &But_2, &But_3, &But_4, &But_5, &But_6, &But_7, &But_8, &But_9, &But_10, &But_11, &But_12};

void keypad_cb(uint8_t i)
{
  Serial.print("Key: ");
  Serial.print(i);
  Serial.println(" pressed");
}

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
  tft.println(SCREEN_ROT * 90);
  tft.print("Size size (WXH):");
  tft.print(TFT_W);
  tft.print("X");
  tft.println(TFT_H);
}
void create_buttons()
{
  const uint8_t txt_size = 3;
  const uint8_t but_size = 60;
  const uint8_t but_space = but_size + 5;
  const uint8_t x_margin = (int)((TFT_W - 3 * but_space) / 2) + but_size / 2;
  const uint8_t y_margin = (int)((TFT_H - 4 * but_space) / 2) + but_size / 2;
  const char *txt_buttons[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};

  for (uint8_t r = 0; r < 4; r++)
  {
    for (uint8_t c = 0; c < 3; c++)
    {
      buttons[3 * r + c]->init();
      buttons[3 * r + c]->screen_rotation = SCREEN_ROT;
      strcpy(buttons[3 * r + c]->txt_buf, txt_buttons[3 * r + c]);
      buttons[3 * r + c]->txt_size = txt_size;
      buttons[3 * r + c]->a = but_size;
      buttons[3 * r + c]->b = but_size;
      buttons[3 * r + c]->xc = x_margin + c * but_space;
      buttons[3 * r + c]->yc = y_margin + r * but_space;
      buttons[3 * r + c]->drawButton();
    }
  }
}
void buttons_press()
{
  for (uint8_t i = 0; i < 12; i++)
  {
    if (buttons[i]->wait4press())
    {
      keypad_cb(i);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  start_GUI();
  GUI_properties();
  create_buttons();
}

void loop()
{
  buttons_press();
}
/* 17 char of size 1 @ 100 pixels ;6 line @ 50 chars*/

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
void calibrate()
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
