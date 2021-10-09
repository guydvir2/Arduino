#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

#define SCREEN_ROT 0
#define RESET_KEYPAD_TIMEOUT 10 // seconds

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

ButtonTFT generic_but_0(ts, tft);
ButtonTFT generic_but_1(ts, tft);
ButtonTFT generic_but_2(ts, tft);
ButtonTFT generic_but_3(ts, tft);
ButtonTFT generic_but_4(ts, tft);
ButtonTFT generic_but_5(ts, tft);
ButtonTFT generic_but_6(ts, tft);
ButtonTFT generic_but_7(ts, tft);
ButtonTFT generic_but_8(ts, tft);
ButtonTFT generic_but_9(ts, tft);
ButtonTFT generic_but_10(ts, tft);
ButtonTFT generic_but_11(ts, tft);
MessageTFT MessageBox(tft);

ButtonTFT *buttons[] = {&generic_but_0, &generic_but_1, &generic_but_2, &generic_but_3, &generic_but_4, &generic_but_5,
                        &generic_but_6, &generic_but_7, &generic_but_8, &generic_but_9, &generic_but_10, &generic_but_11};

int current_menu = 0;
int keypad_value;
char stored_keypad_value[15];

void clearScreen(uint8_t c = 0)
{
  if (c == 0)
  {
    tft.fillScreen(ILI9341_YELLOW);
  }
  else if (c == 1)
  {
    tft.fillScreen(ILI9341_BLACK);
  }
  else if (c == 2)
  {
    tft.fillScreen(ILI9341_BLUE);
  }
}
void start_GUI()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  clearScreen(1);
}

void create_buttons(uint8_t R, uint8_t C, char *but_txt[], uint8_t txt_size = 2)
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
      buttons[C * r + c]->screen_rotation = SCREEN_ROT;
      buttons[C * r + c]->text(but_txt[C * r + c]);
      buttons[C * r + c]->txt_size = txt_size;
      buttons[C * r + c]->a = but_size_a;
      buttons[C * r + c]->b = but_size_b;
      buttons[C * r + c]->xc = x_margin + c * (but_size_a + but_space);
      buttons[C * r + c]->yc = y_margin + r * (but_size_b + but_space);
      buttons[C * r + c]->roundRect = true;
      buttons[C * r + c]->latchButton = false;
      buttons[C * r + c]->drawButton();
    }
  }
}
void create_msg(char *inmsg, uint8_t a = 200, uint8_t b = 50, uint8_t txt_size = 2)
{
  MessageBox.a = 200;
  MessageBox.b = 50;
  MessageBox.xc = tft.width() / 2;
  MessageBox.yc = tft.height() / 2;
  MessageBox.txt_size = txt_size;
  MessageBox.screen_rotation = SCREEN_ROT;
  MessageBox.border_thickness = 1;
  MessageBox.face_color = ILI9341_BLUE;
  MessageBox.border_color = ILI9341_RED;
  MessageBox.txt_color = ILI9341_BLACK;
  MessageBox.roundRect = false;

  MessageBox.text(inmsg);
  MessageBox.drawMSG();
}
void create_keypad()
{
  current_menu = 0;
  clearScreen(2);
  char *txt_buttons[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
  create_buttons(4, 3, txt_buttons);
}

void reset_keypad_values()
{
  strcpy(stored_keypad_value, "");
}
void loop_keypad(TS_Point &p, uint8_t num_items)
{
  for (uint8_t i = 0; i < num_items; i++)
  {
    if (buttons[i]->checkPress(p))
    {
      Serial.println(buttons[i]->txt_buf);
      if (i == 9) /* Erase buffer */
      {
        reset_keypad_values();
        clearScreen(1);
        create_msg("Erased");
        delay(1000);
        create_keypad();
      }
      else if (i == 11) /* Send cmd number */
      {
        create_msg(stored_keypad_value);
        delay(1000);
        reset_keypad_values();
        create_keypad();
      }
      else
      {
        strcat(stored_keypad_value, buttons[i]->txt_buf);
        Serial.println(stored_keypad_value);
      }
    }
  }
}

void setup()
{
  start_GUI();
  create_keypad();
}
void loop()
{
  static unsigned long last_touch = 0;
  if (millis() - last_touch > 500)
  {
    if (ts.touched())
    {
      last_touch = millis();
      TS_Point p = ts.getPoint();
      if (current_menu == 0)
      {
        loop_keypad(p, 12);
      }
    }
    else if (millis() - last_touch > RESET_KEYPAD_TIMEOUT * 1000 && strcmp(stored_keypad_value, "") != 0)
    {
      reset_keypad_values();
      create_keypad();
    }
  }
}
