#include "TFT_GUI.h"

ButtonTFT::ButtonTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft) : _MSGwindow(_tft)
{
}
void ButtonTFT::init()
{
  _MSGwindow.init();
}
void ButtonTFT::drawButton()
{
  _MSGwindow.drawMSG();
}
bool ButtonTFT::wait4press()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    if (_check_press_geometry(p))
    {
      _press_cb();
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
void ButtonTFT::_press_cb()
{
  uint16_t _old_fc = face_color;
  uint16_t _old_bc = border_color;
  face_color = border_color;
  border_color = _old_fc;

  drawButton();
  delay(300);
  face_color = _old_fc;
  border_color = _old_bc;
  drawButton();
  delay(press_delay - 300);
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
    return map(px, TS_MAX_X, TS_MIN_X, 0, _TFT_H);
  }
  else if (screen_rotation == 1)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, _TFT_W);
  }
  else if (screen_rotation == 2)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, _TFT_H);
  }
  else if (screen_rotation == 3)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, _TFT_W);
  }
}
int ButtonTFT::_TS2TFT_y(int py)
{

  if (screen_rotation == 0)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, _TFT_W);
  }
  else if (screen_rotation == 1)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, _TFT_H);
  }
  else if (screen_rotation == 2)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, _TFT_W);
  }
  else if (screen_rotation == 3)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, _TFT_H);
  }
}


MessageTFT::MessageTFT(Adafruit_ILI9341 &_tft)
{
}
void MessageTFT::init()
{
  _TFT_W = tft.width();
  _TFT_H = tft.height();
}
void MessageTFT::drawMSG()
{
  _construct_GUI();
  _put_text();
}
void MessageTFT::_construct_GUI()
{
  tft.setCursor(xc - _pos_corr_factor[txt_size - 1] - a / 2, yc - _pos_corr_factor[txt_size - 1]);
  tft.fillRect(xc - a / 2, yc - b / 2, a, b, face_color);
  for (uint8_t x = 0; x < border_thickness * 2; x++)
  {
    tft.drawRect((xc - a / 2) + x / 2, (yc - b / 2) + x / 2, a - x, b - x, border_color);
  }
}
void MessageTFT::_put_text()
{
  uint8_t x = strlen(txt_buf);
  tft.setCursor(xc - x * _pos_corr_factor[txt_size - 1], yc - _pos_corr_factor[txt_size - 1]);
  tft.setTextColor(txt_color);
  tft.setTextSize(txt_size);
  tft.print(txt_buf);
}
