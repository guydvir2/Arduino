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
keypadTFT keypad(ts,tft);

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
}

void setup()
{
  Serial.begin(115200);
  start_GUI();
  keypad.screen_rotation = SCREEN_ROT;
  keypad.create_keypad();
}

void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    if (keypad.when_pressed(p)) /* Returns true only when code is ended by "#" */
    {
      Serial.println(keypad.keypad_value);
      // clearScreen(2);
      // msg_box(keypad.keypad_value, 200, 40);
      // delay(2000);
      // clearScreen();
      // keypad.create_keypad();
    }
  }
}
