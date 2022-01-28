#include "TFT_GUI.h"

MessageTFT::MessageTFT(Adafruit_ILI9341 &_tft)
{
  TFT[0] = &_tft;
}
void MessageTFT::createMSG(char *txt, bool center_txt)
{
  screen_rotation = TFT[0]->getRotation();
  _drawFace();
  _put_text(txt, txt_color, center_txt);
}
void MessageTFT::createPage(char *txt[], uint8_t r, bool center_txt)
{
  createMSG("", false);
  const uint8_t shiftxy = 5;
  for (uint8_t i = 0; i < r; i++) // multiple lines
  {
    TFT[0]->setCursor(TFT[0]->getCursorX() + shiftxy, TFT[0]->getCursorY() + shiftxy);
    TFT[0]->println(txt[i]);
  }
}
void MessageTFT::updateTXT(char *txt, bool center_txt)
{
  _put_text(txt_buf, face_color, center_txt); // faster to change color txt to face color, than redraw (mostly for clks)
  _put_text(txt, txt_color, center_txt);
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
void MessageTFT::_drawFace()
{
  const uint8_t _radius = 5;

  if (roundRect == false)
  {
    TFT[0]->fillRect(xc - a / 2, yc - b / 2, a, b, face_color);
  }
  else
  {
    TFT[0]->fillRoundRect(xc - a / 2, yc - b / 2, a, b, _radius, face_color);
  }

  if (face_color != border_color)
  {
    _drawBorder(_radius);
  }
}
void MessageTFT::_drawBorder(uint8_t _radius)
{
  for (uint8_t t = 0; t < border_thickness * 2; t++)
  {
    if (roundRect == false)
    {
      TFT[0]->drawRect((xc - a / 2) + t / 2, (yc - b / 2) + t / 2, a - t, b - t, border_color); /* how to change border direction ?*/
    }
    else
    {
      TFT[0]->drawRoundRect((xc - a / 2) + t / 2, (yc - b / 2) + t / 2, a - t, b - t, _radius, border_color);
    }
  }
}
void MessageTFT::_put_text(char *txt, uint16_t color, bool center_txt)
{
  strcpy(txt_buf, txt);
  uint8_t x = strlen(txt_buf);
  if (center_txt)
  {
    TFT[0]->setCursor(xc - x * _pos_corr_factor_x * txt_size, yc - _pos_corr_factor_y * txt_size);
  }
  else
  {
    TFT[0]->setCursor(xc - a / 2, yc - b / 2);
  }
  if (color != NULL)
  {
    TFT[0]->setTextColor(color);
  }
  else
  {
    TFT[0]->setTextColor(txt_color);
  }
  TFT[0]->setTextSize(txt_size);
  TFT[0]->print(txt_buf);
}

ButtonTFT::ButtonTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
{
  TFT[0] = &_tft;
  TS[0] = &_ts;
}
void ButtonTFT::createButton(char *txt)
{
  createMSG(txt);
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
  uint8_t _press_del = 500; // slow down re-triggerring
  if (latchButton == false)
  {
    _face_color_t = face_color;
    face_color = pressedColor;
    createMSG(txt_buf);
    delay(_press_del);
    face_color = _face_color_t;
    createMSG(txt_buf);
    delay(_press_del);
  }
  else
  {
    if (latchState == true) /* Pressed ON*/
    {
      _face_color_t = face_color;
      face_color = pressedColor;
      createMSG(txt_buf);
      delay(_press_del);
    }
    else /* Pressed Off*/
    {
      face_color = _face_color_t;
      createMSG(txt_buf);
      delay(_press_del);
    }
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
{
  _butarray.butarray[0].TFT[0] = &_tft;
  _butarray.butarray[0].TS[0] = &_ts;
}
void keypadTFT::create_keypad()
{
  char *txt_buttons[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
  _butarray.create_array(4, 3, txt_buttons);
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
    return _check_pressed_in(p); /* true only when passcode is delivered ( not only pressed ) */
  }
  else
  {
    return false;
  }
}
void keypadTFT::_reset_keypad_values()
{
  strcpy(_stored_keypad_value, "");
  counter = 0;
}
bool keypadTFT::_check_pressed_in(TS_Point &p)
{
  uint8_t i = _butarray.checkPress(p);
  if (i != 99)
  {
    if (i == 9) /* Erase buffer */
    {
      _reset_keypad_values();
      delay(100);
      return false;
    }
    else if (i == 11) /* Send passcode */
    {
      if (strcmp(_stored_keypad_value, "") != 0)
      {
        strcpy(keypad_value, _stored_keypad_value);
        _reset_keypad_values();
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      strcat(_stored_keypad_value, _butarray.butarray[i].txt_buf);
      counter = strlen(_stored_keypad_value);
      return false;
    }
  }
  else
  {
    false;
  }
}
