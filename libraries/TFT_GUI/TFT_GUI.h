#ifndef TFT_GUI_h
#define TFT_GUI_h

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

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

const uint8_t _pos_corr_factor_x = 3;
const uint8_t _pos_corr_factor_y = 4;

extern XPT2046_Touchscreen ts; /* Touch screen */
extern Adafruit_ILI9341 tft;   /* Graphics */

class MessageTFT
{
public:
  int xc = 0;
  int yc = 0;
  uint8_t a = 0;
  uint8_t b = 0;
  uint8_t txt_size = 2;
  uint8_t border_thickness = 1;
  uint8_t screen_rotation = 0;
  uint16_t face_color = ILI9341_GREENYELLOW;
  uint16_t txt_color = ILI9341_BLACK;
  uint16_t border_color = ILI9341_RED;
  char txt_buf[30];
  bool roundRect = false;
  bool useBorder = false;

  Adafruit_ILI9341 *TFT[1];

public:
  MessageTFT(Adafruit_ILI9341 &_tft = tft);
  void createMSG(const char *txt, bool center_txt = true);
  void createPage(const char *txt[], uint8_t r, bool center_txt = true); // Multiple Lines in MSG
  void updateTXT(const char *txt, bool center_txt = true);
  void clear_screen(uint8_t c = 0);

private:
  void _put_text(const char *txt, uint16_t color = 0, bool center_txt = true);
  void _drawFace();
  void _drawBorder(uint8_t _radius);
};

class ButtonTFT : public MessageTFT
{
public:
  bool latchState = false;
  bool latchButton = false;
  uint16_t pressedColor = ILI9341_RED;
  XPT2046_Touchscreen *TS[1];

public:
  ButtonTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void createButton(const char *txt);
  bool wait4press();
  bool checkPress(TS_Point &p);

private:
  int _tft_x, _tft_y;
  uint16_t _face_color_t;

private:
  void _put_text();
  void _press_cb();
  void _conv_ts_tft(TS_Point &p);
  bool _check_press_geometry(TS_Point &p);
  int _TS2TFT_x(int px);
  int _TS2TFT_y(int py);
};

/* Button Array as class template */
template <uint8_t N>
class buttonArrayTFT
{
public:
  int8_t dx = 5;         /* define spacing between buttons */
  int8_t dy = 5;         /* define spacing between buttons */
  uint8_t scale_f = 100; /* change the cale of array. 100% take entire screen */
  uint8_t scale_y = 100; /* change the cale of array. 100% take entire screen */

  uint8_t shift_y = 255; /* Shifts in y director*/
  uint8_t shift_x = 255; /* Shifts in x director*/
  int shrink_shift = 0;  /* shrink array in pixels, and shifts up/ down (+/-) */

  uint8_t &a = butarray[0].a;
  uint8_t &b = butarray[0].b;
  uint8_t &txt_size = butarray[0].txt_size;
  uint16_t &txt_color = butarray[0].txt_color;
  uint16_t &border_color = butarray[0].border_color;
  uint16_t &face_color = butarray[0].face_color;
  bool &roundRect = butarray[0].roundRect;
  bool &latchButton = butarray[0].latchButton;

  ButtonTFT butarray[N];

public:
  buttonArrayTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void create_array(uint8_t R, uint8_t C, const char *but_txt[]);
  uint8_t checkPress(TS_Point &p);
  ButtonTFT &operator[](uint8_t index)
  {
    if (index < N)
    {
      return butarray[index];
    }
  }
};

template <uint8_t N>
buttonArrayTFT<N>::buttonArrayTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
{
  for (int i = 0; i < N; i++)
  {
    butarray[i].TS[0] = &ts;
    butarray[i].TFT[0] = &tft;
  }
}

template <uint8_t N>
void buttonArrayTFT<N>::create_array(uint8_t R, uint8_t C, const char *but_txt[])
{
  uint8_t x_margin = 0;
  uint8_t y_margin = 0;
  uint8_t but_size_a = 0;
  uint8_t but_size_b = 0;
  const uint8_t marg_clearance = 0;

  if (butarray[0].a != 0 && butarray[0].b != 0) /* buttons side is defined manually */
  {
    but_size_a = butarray[0].a;
    but_size_b = butarray[0].b;
  }
  else /* auto size, resize, shifted and scle factored */
  {
    but_size_a = (uint8_t)((tft.width() * scale_f / 100 - marg_clearance - dx) / C);
    but_size_b = (uint8_t)((tft.height() * scale_f * scale_y / 100 / 100 - abs(shrink_shift) - marg_clearance - dy) / R);
  }

  if (shrink_shift != 0)
  {
    y_margin = shrink_shift;
    x_margin = (int)(tft.width() + (1 - C) * (but_size_a + dx)) / 2;
  }
  else
  {
    if (shift_x == 255)
    {
      x_margin = (int)(tft.width() * scale_f / 100 + (1 - C) * (but_size_a + dx)) / 2;
    }
    else
    {
      x_margin = shift_x + but_size_a / 2;
    }
    if (shift_y == 255)
    {
      y_margin = (int)(tft.height() * scale_f * scale_y / 100 / 100 + (1 - R) * (but_size_b + dy)) / 2 + shrink_shift;
    }
    else
    {
      y_margin = shift_y + but_size_b / 2;
    }
  }

  for (uint8_t r = 0; r < R; r++)
  {
    for (uint8_t c = 0; c < C; c++)
    {
      butarray[C * r + c].a = but_size_a; /* Calculated*/
      butarray[C * r + c].b = but_size_b; /* Calculated*/
      butarray[C * r + c].xc = x_margin + c * (but_size_a + dx);
      butarray[C * r + c].yc = y_margin + r * (but_size_b + dy);
      butarray[C * r + c].txt_size = txt_size;
      butarray[C * r + c].txt_color = txt_color;
      butarray[C * r + c].border_color = border_color;
      butarray[C * r + c].face_color = face_color;
      butarray[C * r + c].roundRect = roundRect;
      butarray[C * r + c].latchButton = latchButton;

      butarray[C * r + c].createButton(but_txt[C * r + c]);
    }
  }
}

template <uint8_t N>
uint8_t buttonArrayTFT<N>::checkPress(TS_Point &p)
{
  if (butarray[0].TS[0]->touched()) // check in any press occured
  {
    for (uint8_t i = 0; i < N; i++)
    {
      if (butarray[i].checkPress(p))
      {
        return i;
      }
    }
    return 99;
  }
  else
  {
    return 99;
  }
}
/* End of template */

class keypadTFT
{
#define RESET_KEYPAD_TIMEOUT 10 // seconds

public:
  keypadTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void create_keypad();
  bool getPasscode(TS_Point &p);

protected:
  buttonArrayTFT<12> _butarray;

public:
  uint8_t counter = 0;
  uint8_t &scale_f = _butarray.scale_f;
  uint8_t &shift_y = _butarray.shift_y;
  uint8_t &shift_x = _butarray.shift_x;
  uint8_t &txt_size = _butarray.butarray[0].txt_size;
  int &shrink_shift = _butarray.shrink_shift;
  uint16_t &txt_color = _butarray.butarray[0].txt_color;
  uint16_t &border_color = _butarray.butarray[0].border_color;
  uint16_t &face_color = _butarray.butarray[0].face_color;
  bool &roundRect = _butarray.butarray[0].roundRect;
  char keypad_value[15]; /* To reach externally */

private:
  void _create_buttons(uint8_t R, uint8_t C, char *but_txt[]);
  void _reset_keypad_values();
  bool _check_pressed_in(TS_Point &p);

private:
  char _stored_keypad_value[15];
};

#endif
/*
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198
*/