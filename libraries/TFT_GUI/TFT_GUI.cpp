#include "TFT_GUI.h"

MessageTFT::MessageTFT(Adafruit_ILI9341 &_tft)
{
  TFT[0] = &_tft;
}
void MessageTFT::drawMSG(char *txt, uint8_t a, uint8_t b, int xc, int yc, uint8_t txt_size, uint8_t border_thickness, uint16_t face_color, uint16_t border_color, uint16_t txt_color, bool roundRect)
{
  screen_rotation = TFT[0]->getRotation();
  _drawFace(a, b, xc, yc, face_color, roundRect);
  _drawBorder(a, b, xc, yc, border_thickness, border_color, roundRect);
  _put_text(txt, xc, yc, txt_size, txt_color);
}
void MessageTFT::clear_screen(uint8_t c)
{
  if (c == 0)
  {
    TFT[0]->fillScreen(ILI9341_BLACK);
  }
  else if (c == 1)
  {
    TFT[0]->fillScreen(ILI9341_YELLOW);
  }
  else if (c == 2)
  {
    TFT[0]->fillScreen(ILI9341_BLUE);
  }
}
void MessageTFT::_drawFace(uint8_t a, uint8_t b, int xc, int yc, uint16_t face_color, bool roundRect)
{
  const uint8_t _radius = 15;

  if (roundRect == false)
  {
    TFT[0]->fillRect(xc - a / 2, yc - b / 2, a, b, face_color);
  }
  else
  {
    TFT[0]->fillRoundRect(xc - a / 2, yc - b / 2, a, b, a / _radius, face_color);
  }
}
void MessageTFT::_drawBorder(uint8_t a, uint8_t b, int xc, int yc, uint8_t border_thickness, uint16_t border_color, bool roundRect)
{
  const uint8_t _radius = 15;

  for (uint8_t t = 0; t < border_thickness * 2; t++)
  {
    if (roundRect == false)
    {
      TFT[0]->drawRect((xc - a / 2) + t / 2, (yc - b / 2) + t / 2, a - t, b - t, border_color); /* how to change border direction ?*/
    }
    else
    {
      TFT[0]->drawRoundRect((xc - a / 2) + t / 2, (yc - b / 2) + t / 2, a - t, b - t, a / _radius, border_color);
    }
  }
}
void MessageTFT::_put_text(char *txt, int xc, int yc, uint8_t txt_size, uint16_t txt_color)
{
  strcpy(txt_buf, txt);
  uint8_t x = strlen(txt_buf);
  TFT[0]->setCursor(xc - x * _pos_corr_factor_x * txt_size, yc - _pos_corr_factor_y * txt_size);
  TFT[0]->setTextColor(txt_color);
  TFT[0]->setTextSize(txt_size);
  TFT[0]->print(txt_buf);
}

ButtonTFT::ButtonTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft) : MSGwindow(_tft)
{
  MSGwindow.TFT[0] = &_tft;
  TS[0] = &_ts;
}
void ButtonTFT::drawButton(char *txt, uint8_t _a, uint8_t _b, int _xc, int _yc, uint8_t _txt_size, uint16_t _face_color, uint16_t _border_color, uint16_t _txt_color)
{
  a = _a;
  b = _b;
  xc = _xc;
  yc = _yc;
  txt_size = _txt_size;
  txt_color = _txt_color;
  face_color = _face_color;
  border_color = _border_color;
  MSGwindow.drawMSG(txt, a, b, xc, yc, txt_size, 1, face_color, border_color, txt_color, true);
}
bool ButtonTFT::wait4press() /* include getPoint loop - use for simple cases*/
{
  if (TS[0]->touched())
  {
    TS_Point p = TS[0]->getPoint();
    return checkPress(p); /* in or out ? */
  }
  else
  {
    return 0;
  }
}
bool ButtonTFT::checkPress(TS_Point &p) /* can be called from code outside lib when lots of buttons involved */
{
  if (_check_press_geometry(p))
  {
    if (latchButton)
    {
      latchState = !latchState;
    }
    _press_cb();
    return 1;
  }
  else
  {
    return 0;
  }
}
void ButtonTFT::_press_cb()
{
  uint8_t _press_del = 80;
  if (latchButton == false)
  {
    MSGwindow.drawMSG(txt_buf, a, b, xc, yc, txt_size, 1, ILI9341_RED, border_color, txt_color, true);
    delay(_press_del);
    MSGwindow.drawMSG(txt_buf, a, b, xc, yc, txt_size, 1, face_color, border_color, txt_color, true);
    delay(_press_del);
  }
  else
  {
    //   if (latchState == true) /* Pressed ON*/
    //   {
    //     tempColor = face_color;
    //     face_color = ILI9341_RED;
    //     drawButton();
    //     delay(_press_del);
    //   }
    //   else /* Pressed Off*/
    //   {
    //     face_color = tempColor;
    //     drawButton();
    //     delay(_press_del);
    // }
  }
}
bool ButtonTFT::_check_press_geometry(TS_Point &p)
{
  _conv_ts_tft(p);
  if (_tft_x <= xc + a / 2 && _tft_x >= xc - a / 2)
  {
    if (_tft_y <= yc + b / 2 && _tft_y >= yc - b / 2)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}
void ButtonTFT::_conv_ts_tft(TS_Point &p)
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
int ButtonTFT::_TS2TFT_x(int px)
{
  if (screen_rotation == 0)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, tft.height());
  }
  else if (screen_rotation == 1)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, tft.width());
  }
  else if (screen_rotation == 2)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, tft.height());
  }
  else if (screen_rotation == 3)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, tft.width());
  }
  else
  {
    return 9999;
  }
}
int ButtonTFT::_TS2TFT_y(int py)
{
  if (screen_rotation == 0)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, tft.width());
  }
  else if (screen_rotation == 1)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, tft.height());
  }
  else if (screen_rotation == 2)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, tft.width());
  }
  else if (screen_rotation == 3)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, tft.height());
  }
  else
  {
    return 99999999;
  }
}

keypadTFT::keypadTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
    : _button0(_ts, _tft), _button1(_ts, _tft), _button2(_ts, _tft),
      _button3(_ts, _tft), _button4(_ts, _tft), _button5(_ts, _tft),
      _button6(_ts, _tft), _button7(_ts, _tft), _button8(_ts, _tft),
      _button9(_ts, _tft), _button_10(_ts, _tft), _button_11(_ts, _tft)
{
  _button0.MSGwindow.TFT[0] = &_tft;
  _button0.TS[0] = &_ts;
}
void keypadTFT::create_keypad()
{
  char *txt_buttons[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
  _create_buttons(4, 3, txt_buttons);
}
void keypadTFT::_create_buttons(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size)
{
  const uint8_t but_space = 5;
  const uint8_t but_size_a = (uint8_t)((tft.width() - 50) / C);
  const uint8_t but_size_b = (uint8_t)((tft.height() - 50) / R);
  const uint8_t x_margin = (int)(tft.width() + (1 - C) * (but_size_a + but_space)) / 2;
  const uint8_t y_margin = (int)(tft.height() + (1 - R) * (but_size_b + but_space)) / 2;

  for (uint8_t r = 0; r < R; r++)
  {
    for (uint8_t c = 0; c < C; c++)
    {
      _buttons[C * r + c]->MSGwindow.TFT[0] = _button0.MSGwindow.TFT[0];
      _buttons[C * r + c]->TS[0] = _button0.TS[0];
      _buttons[C * r + c]->drawButton(but_txt[C * r + c], but_size_a, but_size_b, x_margin + c * (but_size_a + but_space), y_margin + r * (but_size_b + but_space), txt_size);
    }
  }
}
void keypadTFT::_reset_keypad_values()
{
  strcpy(_stored_keypad_value, "");
}
bool keypadTFT::_check_pressed_in(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (_buttons[i]->checkPress(p))
    {
      Serial.println(_buttons[i]->txt_buf);
      if (i == 9) /* Erase buffer */
      {
        _reset_keypad_values();
        delay(1000);
        create_keypad();
        return false;
      }
      else if (i == 11) /* Send passcode */
      {
        if (strcmp(_stored_keypad_value, "") != 0)
        {
          strcpy(keypad_value, _stored_keypad_value);
          _reset_keypad_values();
          create_keypad();
          return true;
        }
        else
        {
          return false;
        }
      }
      else
      {
        strcat(_stored_keypad_value, _buttons[i]->txt_buf);
        Serial.println(_stored_keypad_value);
        return false;
      }
    }
  }
  return false;
}
bool keypadTFT::getPasscode(TS_Point &p)
{
  static unsigned long last_touch = 0;
  if (millis() - last_touch > 500)
  {
    if (millis() - last_touch > RESET_KEYPAD_TIMEOUT * 1000 && strcmp(_stored_keypad_value, "") != 0) /* Clear buffer after timeout */
    {
      _reset_keypad_values();
    }
    last_touch = millis();
    return _check_pressed_in(p, 12); /* true only when passcode is delivered ( not only pressed ) */
  }
  else
  {
    return false;
  }
}

buttonArrayTFT::buttonArrayTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
    : _button0(_ts, _tft), _button1(_ts, _tft), _button2(_ts, _tft),
      _button3(_ts, _tft), _button4(_ts, _tft), _button5(_ts, _tft),
      _button6(_ts, _tft), _button7(_ts, _tft)
{
}
uint8_t buttonArrayTFT::checkPress(TS_Point &p)
{
  for (uint8_t i = 0; i < _num_items; i++)
  {
    if (_buttons[i]->checkPress(p))
    {
      return i;
    }
  }
  return 99;
}
void buttonArrayTFT::create_array(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size, uint16_t face_c, uint16_t border_c, uint16_t text_c)
{
  _num_items = R * C;
  const uint8_t but_space = 5;
  const uint8_t but_size_a = (uint8_t)((tft.width() - 50) / C);
  const uint8_t but_size_b = (uint8_t)((tft.height() - 50) / R);
  const uint8_t x_margin = (int)(tft.width() + (1 - C) * (but_size_a + but_space)) / 2;
  const uint8_t y_margin = (int)(tft.height() + (1 - R) * (but_size_b + but_space)) / 2;

  for (uint8_t r = 0; r < R; r++)
  {
    for (uint8_t c = 0; c < C; c++)
    {
      _buttons[C * r + c]->MSGwindow.TFT[0] = _button0.MSGwindow.TFT[0];
      _buttons[C * r + c]->TS[0] = _button0.TS[0];
      _buttons[C * r + c]->drawButton(but_txt[C * r + c], but_size_a, but_size_b, x_margin + c * (but_size_a + but_space), y_margin + r * (but_size_b + but_space), txt_size, face_c, border_c, text_c);
    }
  }
}
