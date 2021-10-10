#ifndef TFT_GUI_h
#define TFT_GUI_h

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

extern XPT2046_Touchscreen ts; /* Touch screen */
extern Adafruit_ILI9341 tft;   /* Graphics */

const uint8_t _pos_corr_factor_x = 3;
const uint8_t _pos_corr_factor_y = 4;

/* Screen calibration */
#define TS_MIN_X 350
#define TS_MIN_Y 350
#define TS_MAX_X 3800
#define TS_MAX_Y 3800
/* ~~~~~~~~~~~~~~~~~ */

/* For Wemos Mini and TFT screen 2.4" */
#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class MessageTFT
{
public:
  int xc, yc;
  uint8_t a, b;
  uint8_t txt_size;
  char txt_buf[30];
  uint8_t screen_rotation = 0;
  Adafruit_ILI9341 *TFT[1];

public:
  MessageTFT(Adafruit_ILI9341 &_tft = tft);
  void drawMSG(char *txt, uint8_t a, uint8_t b, int xc, int yc, uint8_t txt_size = 2, uint8_t border_thickness = 1, uint16_t face_color = ILI9341_GREEN, uint16_t border_color = ILI9341_RED, uint16_t txt_color = ILI9341_BLACK, bool roundRect = true);
  void clear_screen(uint8_t c = 0);

private:
  void _put_text(char *txt, int xc, int yc, uint8_t txt_size, uint16_t txt_color);
  void _drawFace(uint8_t a, uint8_t b, int xc, int yc, uint16_t face_color, bool roundRect);
  void _drawBorder(uint8_t a, uint8_t b, int xc, int yc, uint8_t border_thickness, uint16_t border_color, bool roundRect);
};

class ButtonTFT
{
public:
  int &xc = MSGwindow.xc;
  int &yc = MSGwindow.yc;
  uint8_t &a = MSGwindow.a;
  uint8_t &b = MSGwindow.b;
  uint8_t &txt_size = MSGwindow.txt_size;
  uint8_t &screen_rotation = MSGwindow.screen_rotation;
  char *txt_buf = MSGwindow.txt_buf;

  bool latchState = false;
  bool latchButton = false;
  uint16_t face_color;
  uint16_t border_color;
  uint16_t txt_color;

  XPT2046_Touchscreen *TS[1];
  MessageTFT MSGwindow;

public:
  ButtonTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void drawButton(char *txt, uint8_t _a, uint8_t _b, int _xc, int _yc, uint8_t _txt_size = 2, uint16_t _face_color = ILI9341_GREEN, uint16_t _border_color = ILI9341_RED, uint16_t _txt_color = ILI9341_BLACK);
  bool wait4press();
  bool checkPress(TS_Point &p);

private:
  int _tft_x, _tft_y;

private:
  void _construct_button();
  void _put_text();
  void _press_cb();
  void _conv_ts_tft(TS_Point &p);
  bool _check_press_geometry(TS_Point &p);
  int _TS2TFT_x(int px);
  int _TS2TFT_y(int py);
};

class keypadTFT
{
#define RESET_KEYPAD_TIMEOUT 10 // seconds

public:
  keypadTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void create_keypad();
  bool getPasscode(TS_Point &p);

public:
  char keypad_value[15]; /* To reach externally */

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
  bool _check_pressed_in(TS_Point &p, uint8_t num_items);

private:
  char _stored_keypad_value[15];
};

class buttonArrayTFT
{
public:
  buttonArrayTFT(XPT2046_Touchscreen &_ts=ts, Adafruit_ILI9341 &_tft=tft);
  void create_array(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size = 2, uint16_t face_c = ILI9341_GREEN, uint16_t border_c = ILI9341_RED, uint16_t text_c = ILI9341_BLACK);
  uint8_t checkPress(TS_Point &p);

public:
  char txt_buf[30];

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
