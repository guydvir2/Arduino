#ifndef TFT_GUI_h
#define TFT_GUI_h

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// #define GLCD_CL_BLACK 0x0000
// #define GLCD_CL_WHITE 0xFFFF
// #define GLCD_CL_GRAY 0x7BEF
// #define GLCD_CL_LIGHT_GRAY 0xC618
// #define GLCD_CL_GREEN 0x07E0
// #define GLCD_CL_LIME 0x87E0
// #define GLCD_CL_BLUE 0x001F
// #define GLCD_CL_RED 0xF800
// #define GLCD_CL_AQUA 0x5D1C
// #define GLCD_CL_YELLOW 0xFFE0
// #define GLCD_CL_MAGENTA 0xF81F
// #define GLCD_CL_CYAN 0x07FF
// #define GLCD_CL_DARK_CYAN 0x03EF
// #define GLCD_CL_ORANGE 0xFCA0
// #define GLCD_CL_PINK 0xF97F
// #define GLCD_CL_BROWN 0x8200
// #define GLCD_CL_VIOLET 0x9199
// #define GLCD_CL_SILVER 0xA510
// #define GLCD_CL_GOLD 0xA508
// #define GLCD_CL_NAVY 0x000F
// #define GLCD_CL_MAROON 0x7800
// #define GLCD_CL_PURPLE 0x780F
// #define GLCD_CL_OLIVE 0x7BE0

extern XPT2046_Touchscreen ts; /* Touch screen */
extern Adafruit_ILI9341 tft;   /* Graphics */

const uint8_t _pos_corr_factor_x = 3;
const uint8_t _pos_corr_factor_y = 4;

class MessageTFT
{
public:
  uint8_t a, b;
  int xc, yc;
  uint8_t txt_size = 1;
  uint8_t screen_rotation = 0;
  uint8_t border_thickness = 1;
  uint16_t face_color = ILI9341_GREEN;
  uint16_t border_color = ILI9341_RED;
  uint16_t txt_color = ILI9341_BLACK;

  char txt_buf[30];
  bool roundRect = false;

public:
  MessageTFT(Adafruit_ILI9341 &_tft);
  void drawMSG();
  void text(char *txt);

private:
  uint8_t _radius = 15;

private:
  void _drawFace();
  void _drawBorder();
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
  int &xc = _MSGwindow.xc;
  int &yc = _MSGwindow.yc;
  uint8_t &a = _MSGwindow.a;
  uint8_t &b = _MSGwindow.b;
  uint8_t &txt_size = _MSGwindow.txt_size;
  uint8_t &border_thickness = _MSGwindow.border_thickness;
  uint8_t &screen_rotation = _MSGwindow.screen_rotation;
  uint16_t &face_color = _MSGwindow.face_color;
  uint16_t &border_color = _MSGwindow.border_color;
  uint16_t &txt_color = _MSGwindow.txt_color;
  uint16_t tempColor;

  bool &roundRect = _MSGwindow.roundRect;
  char *txt_buf = _MSGwindow.txt_buf;

  bool latchButton = false;
  bool latchState = false;
  uint16_t faceColor_pressed = ILI9341_RED;

public:
  ButtonTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft);
  void drawButton();
  void text(char *txt);
  bool wait4press();
  bool checkPress(TS_Point &p);

private:
  int _tft_x, _tft_y;
  uint8_t _press_del = 75;

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

class keypadTFT
{
#define RESET_KEYPAD_TIMEOUT 10 // seconds

public:
  keypadTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft);
  void create_keypad();
  bool when_pressed(TS_Point &p);
  void loop();

public:
  uint8_t screen_rotation = 0;
  uint16_t face_color = ILI9341_GREEN;
  uint16_t border_color = ILI9341_RED;
  uint16_t txt_color = ILI9341_BLACK;
  char keypad_value[15];

private:
  ButtonTFT _button0;
  ButtonTFT _button1;
  ButtonTFT _button2;
  ButtonTFT _button3;
  ButtonTFT _button4;
  ButtonTFT _button5;
  ButtonTFT _button6;
  ButtonTFT _button7;
  ButtonTFT _button8;
  ButtonTFT _button9;
  ButtonTFT _button_10;
  ButtonTFT _button_11;

  ButtonTFT *_buttons[12] = {&_button0, &_button1, &_button2, &_button3, &_button4, &_button5,
                             &_button6, &_button7, &_button8, &_button9, &_button_10, &_button_11};

  void _create_buttons(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size = 2);
  void _reset_keypad_values();
  bool _loop_keypad(TS_Point &p, uint8_t num_items);
  char _stored_keypad_value[15];
};

class buttonArrayTFT
{
public:
  buttonArrayTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft);
  void create_array(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size = 2, uint16_t face_c = ILI9341_GREEN, uint16_t border_c = ILI9341_RED, uint16_t text_c = ILI9341_BLACK);
  uint8_t checkPress(TS_Point &p);

public:
  char txt_buf[30];
  uint8_t screen_rotation = 0;

private:
  ButtonTFT _button0;
  ButtonTFT _button1;
  ButtonTFT _button2;
  ButtonTFT _button3;
  ButtonTFT _button4;
  ButtonTFT _button5;
  ButtonTFT _button6;
  ButtonTFT _button7;
  ButtonTFT *_buttons[8] = {&_button0, &_button1, &_button2, &_button3,
                             &_button4, &_button5, &_button6, &_button7};

private:
  uint8_t _num_items = 0;
};

#endif
