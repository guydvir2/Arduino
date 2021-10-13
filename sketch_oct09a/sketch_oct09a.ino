#include <TFT_GUI.h>

#define SCREEN_ROT 0

#define CASE 4
#if CASE == 1
MessageTFT MsgBox;
#elif CASE == 2
MessageTFT dataBox;
MessageTFT dataBox;
ButtonTFT butt;
#elif CASE == 3
keypadTFT keypad;
#elif CASE == 4
buttonArrayTFT buttArray;
#endif

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

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
  Serial.begin(115200);
  start_touchScreen();

#if CASE == 1
  MsgBox.drawMSG("msgBox", 200, 60, tft.width() / 2, 25);
#elif CASE == 2
  butt.drawButton("Button", 150, 50, tft.width() / 2, tft.height() / 2);
#elif case == 3
  keypad.create_keypad();
#elif case == 4
  buttArray.create_array(3, 2, "F1,F2,F3,F4,F5,F6");
#endif
}

void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
#if CASE == 2
    if (butt.checkPress(p))
    {
      Serial.print("Pressed");
    }
#elif CASE == 3
    if (keypad.getPasscode(p))
    {
      Serial.println(keypad.keypad_value);
    }
#elif CASE == 4
    uint8_t result = buttArray.checkPress(p);
    if (result != 99)
    {
      Serial.print("Pressed: ");
      Serial.println(result);
    }
#endif
    // Serial.println(butArray.checkPress(p));
  }
}
