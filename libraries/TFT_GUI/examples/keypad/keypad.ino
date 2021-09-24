#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3


XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

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

int TFT_W = 0;
int TFT_H = 0;
const uint8_t SCREEN_ROT = 0;

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
  tft.print(SCREEN_ROT * 90);
  tft.print("; Size (WXH):");
  tft.print(TFT_W);
  tft.print("X");
  tft.print(TFT_H);
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