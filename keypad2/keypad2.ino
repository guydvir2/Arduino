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

const uint8_t SCREEN_ROT = 1;

void keypad_cb(char *passcode)
{
  Serial.print("passcode: ");
  Serial.print(passcode);
}

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
void start_GUI()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
  clearScreen();
}

void setup()
{
  Serial.begin(115200);
  start_GUI();
  keypad.create_keypad();
}

void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    if (keypad.getPasscode(p)) /* Returns true only when code is ended by "#" */
    {
      keypad_cb(keypad.keypad_value);
    }
  }
}
