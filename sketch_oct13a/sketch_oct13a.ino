#include <TFT_GUI.h>
#define SCREEN_ROT 0

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// template <uint8_t N>
// class buttonArray_TFT
// {
// public:
//   int8_t dx = 5;         /* define spacing between buttons */
//   int8_t dy = 5;         /* define spacing between buttons */
//   uint8_t scale_f = 100; /* change the cale of array. 100% take entire screen */
//   uint8_t shift_y = 255; /* Shifts in y director*/
//   uint8_t shift_x = 255; /* Shifts in x director*/
//   int shrink_shift = 0;  /* shrink array in pixels, and shifts up/ down (+/-) */
  
//   uint8_t &a = butarray[0].a;
//   uint8_t &b = butarray[0].b;
//   uint8_t &txt_size = butarray[0].txt_size;
//   uint16_t &txt_color = butarray[0].txt_color;
//   uint16_t &border_color = butarray[0].border_color;
//   uint16_t &face_color = butarray[0].face_color;
//   bool &roundRect = butarray[0].roundRect;

// protected:
//   ButtonTFT butarray[N];

// public:
//   buttonArray_TFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft)
//   {
//     for (int i = 0; i < N; i++)
//     {
//       butarray[i].TS[0] = &ts;
//       butarray[i].TFT[0] = &tft;
//     }
//   }
//   void create_array(uint8_t R, uint8_t C, char *but_txt[])
//   {
//     uint8_t x_margin = 0;
//     uint8_t y_margin = 0;
//     uint8_t but_size_a = 0;
//     uint8_t but_size_b = 0;
//     const uint8_t marg_clearance = 10;

//     if (butarray[0].a != 0 && butarray[0].b != 0) /* buttons side is defined manually */
//     {
//       but_size_a = butarray[0].a;
//       but_size_b = butarray[0].b;
//     }
//     else /* auto size, resize, shifted and scle factored */
//     {
//       but_size_a = (uint8_t)((tft.width() * scale_f / 100 - marg_clearance) / C);
//       but_size_b = (uint8_t)((tft.height() * scale_f / 100 - abs(shrink_shift) - marg_clearance) / R);
//     }

//     if (shrink_shift != 0)
//     {
//       y_margin = shrink_shift;
//     }
//     else
//     {
//       if (shift_x == 255)
//       {
//         x_margin = (int)(tft.width() + (1 - C) * (but_size_a + dx)) / 2;
//       }
//       else
//       {
//         x_margin = shift_x + but_size_a / 2;
//       }
//       if (shift_y == 255)
//       {
//         y_margin = (int)(tft.height() + (1 - R) * (but_size_b + dy)) / 2 + shrink_shift;
//       }
//       else
//       {
//         y_margin = shift_y + but_size_b / 2;
//       }
//     }

//     for (uint8_t r = 0; r < R; r++)
//     {
//       for (uint8_t c = 0; c < C; c++)
//       {
//         butarray[C * r + c].a = but_size_a; /* Calculated*/
//         butarray[C * r + c].b = but_size_b; /* Calculated*/
//         butarray[C * r + c].xc = x_margin + c * (but_size_a + dx);
//         butarray[C * r + c].yc = y_margin + r * (but_size_b + dy);
//         butarray[C * r + c].txt_size = txt_size;
//         butarray[C * r + c].txt_color = txt_color;
//         butarray[C * r + c].border_color = border_color;
//         butarray[C * r + c].face_color = face_color;
//         butarray[C * r + c].roundRect = roundRect;

//         butarray[C * r + c].createButton(but_txt[C * r + c]);
//       }
//     }
//   }
//   uint8_t checkPress(TS_Point &p)
//   {
//     for (uint8_t i = 0; i < N; i++)
//     {
//       if (butarray[i].checkPress(p))
//       {
//         return i;
//       }
//     }
//     return 99;
//   }
// };
buttonArray_TFT<6> butarr;

void clearScreen(int c = 0)
{
  if (c == 0)
  {
    tft.fillScreen(ILI9341_BLACK);
  }
  else if (c == 1)
  {
    tft.fillScreen(ILI9341_YELLOW);
  }
  else if (c == 2)
  {
    tft.fillScreen(ILI9341_BLUE);
  }
}
void start_touchScreen()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  clearScreen();
}
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  start_touchScreen();
  char *a [6] = {"F1", "F2", "F3", "F4", "F5", "F6"};
  butarr.create_array(3, 2, a);
}

void loop()
{
  // put your main code here, to run repeatedly:
  // if (ts.touched())
  // {
  //   TS_Point p = ts.getPoint();
  //   // butarr.checkPress(p);
  // }
}
