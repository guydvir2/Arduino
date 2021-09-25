#ifndef TFT_GUI_h
#define TFT_GUI_h

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#define GLCD_CL_BLACK 0x0000
#define GLCD_CL_WHITE 0xFFFF
#define GLCD_CL_GRAY 0x7BEF
#define GLCD_CL_LIGHT_GRAY 0xC618
#define GLCD_CL_GREEN 0x07E0
#define GLCD_CL_LIME 0x87E0
#define GLCD_CL_BLUE 0x001F
#define GLCD_CL_RED 0xF800
#define GLCD_CL_AQUA 0x5D1C
#define GLCD_CL_YELLOW 0xFFE0
#define GLCD_CL_MAGENTA 0xF81F
#define GLCD_CL_CYAN 0x07FF
#define GLCD_CL_DARK_CYAN 0x03EF
#define GLCD_CL_ORANGE 0xFCA0
#define GLCD_CL_PINK 0xF97F
#define GLCD_CL_BROWN 0x8200
#define GLCD_CL_VIOLET 0x9199
#define GLCD_CL_SILVER 0xA510
#define GLCD_CL_GOLD 0xA508
#define GLCD_CL_NAVY 0x000F
#define GLCD_CL_MAROON 0x7800
#define GLCD_CL_PURPLE 0x780F
#define GLCD_CL_OLIVE 0x7BE0

extern XPT2046_Touchscreen ts; /* Touch screen */
extern Adafruit_ILI9341 tft;   /* Graphics */

const uint8_t _pos_corr_factor[3] = {3, 6, 9}; /* Center text inside a box */
class MessageTFT
{
public:
  uint8_t xc, yc;
  uint8_t a, b;
  uint8_t txt_size = 1;
  uint8_t border_thickness = 1;
  uint8_t screen_rotation = 0;
  uint16_t press_delay = 1000;
  uint16_t face_color = ILI9341_GREEN;
  uint16_t border_color = ILI9341_RED;
  uint16_t txt_color = ILI9341_BLACK;

  char txt_buf[30];

public:
  MessageTFT(Adafruit_ILI9341 &_tft);
  void init();
  void drawMSG();
  void text(char *txt);

private:
  int _TFT_W = 0;
  int _TFT_H = 0;

private:
  void _construct_GUI();
  void _put_text();
};
class ButtonTFT
{
public:
  /* Screen calibration */
  int TS_MIN_X = 350;
  int TS_MIN_Y = 350;
  int TS_MAX_X = 3800;
  int TS_MAX_Y = 3800;
  /* ~~~~~~~~~~~~~~~~~ */
  char txt_buf[15];

  uint8_t &xc = _MSGwindow.xc;
  uint8_t &yc = _MSGwindow.yc;
  uint8_t &a = _MSGwindow.a;
  uint8_t &b = _MSGwindow.b;
  uint8_t &txt_size = _MSGwindow.txt_size;
  uint8_t &border_thickness = _MSGwindow.border_thickness;
  uint8_t &screen_rotation = _MSGwindow.screen_rotation;
  uint16_t &face_color = _MSGwindow.face_color;
  uint16_t &border_color = _MSGwindow.border_color;
  uint16_t &txt_color = _MSGwindow.txt_color;
  uint16_t press_delay = 1000;

public:
  ButtonTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft);
  void init();
  void drawButton();
  void text(char *txt);
  bool wait4press();

private:
  int _TFT_W = 0;
  int _TFT_H = 0;
  int _tft_x, _tft_y;

private:
  void _construct_button();
  void _put_text();
  void _press_cb();
  void _conv_ts_tft(TS_Point &p);
  bool _check_press_geometry(TS_Point &p);
  int _TS2TFT_x(int px);
  int _TS2TFT_y(int py);

  MessageTFT _MSGwindow;
  };
#endif
