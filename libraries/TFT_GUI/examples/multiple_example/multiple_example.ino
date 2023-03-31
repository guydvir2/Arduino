#include <TFT_GUI.h>

#define SCREEN_ROT 0

#define CASE 4

#if CASE == 1 /* Simple MsgBox - No Touch */
MessageTFT MsgBox;

#elif CASE == 2 /* Simple Button - incl Touch */
ButtonTFT butt;

#elif CASE == 3 /* 12 button preDefined Keypad - "*" - Delete enter code before sending "#"- transmit code */
keypadTFT keypad;

#elif CASE == 4 /* Button Array - equally spaced buttons */
const char *a[] = {"All Windows", "Saloon", "Room", "Specific"};
buttonArrayTFT<4> mainWindows;
#elif CASE == 5 /* Shifted Array and 2 single Buttons */
char *a[] = {"F1", "F2"};
buttonArrayTFT<2> shiftedArray;
#elif CASE == 6
newArray<12> array;

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
  MsgBox.a = 200;               // width
  MsgBox.b = 60;                // height
  MsgBox.xc = tft.width() / 2;  // xc position
  MsgBox.yc = tft.height() / 2; // yc position
  MsgBox.txt_size = 2;
  MsgBox.roundRect = true;
  MsgBox.screen_rotation = SCREEN_ROT;
  MsgBox.face_color = ILI9341_GREENYELLOW;
  MsgBox.txt_color = ILI9341_BLACK;
  MsgBox.border_color = ILI9341_RED;
  MsgBox.createMSG("This is msgBox");

#elif CASE == 2
  butt.a = 200;
  butt.b = 60;
  butt.xc = tft.width() / 2;
  butt.yc = tft.height() / 2;
  butt.txt_size = 3;
  butt.face_color = ILI9341_CASET;
  butt.txt_color = ILI9341_WHITE;
  butt.border_color = ILI9341_MAGENTA;
  butt.roundRect = false;
  butt.createButton("Press!");

#elif CASE == 3
  keypad.txt_size = 2;
  keypad.txt_color = ILI9341_WHITE;
  keypad.face_color = ILI9341_LIGHTGREY;
  keypad.border_color = ILI9341_LIGHTGREY;
  keypad.roundRect = false;
  keypad.create_keypad();

#elif CASE == 4
  mainWindows.shift_y = 0;
  mainWindows.shift_x = 30;
  // mainWindows.scale_f = 80;
  mainWindows.a = 150; /* define size manually */
  mainWindows.b = 50;  /* define size manually */
  mainWindows.dy = 10; /* define y spacing between buttons */
  mainWindows.txt_size = 2;
  mainWindows.roundRect = false;
  mainWindows.txt_color = ILI9341_WHITE;
  mainWindows.face_color = ILI9341_DARKGREY;
  mainWindows.border_color = mainWindows.face_color;
  mainWindows.create_array(4, 1, a);

#elif CASE == 5
  /* Define manually button's dimensions and spacings */
  shiftedArray.a = 80;
  shiftedArray.b = 50;
  shiftedArray.dx = 2;
  shiftedArray.dy = 2;
  // shiftedArray.shift_y = 70;
  // shiftedArray.shift_x = 40;
  shiftedArray.txt_size = 2;
  shiftedArray.face_color = ILI9341_GREENYELLOW;
  shiftedArray.txt_color = ILI9341_RED;
  shiftedArray.border_color = ILI9341_GREENYELLOW;
  shiftedArray.roundRect = true;
  // shiftedArray.shrink_shift = -60;
  shiftedArray.create_array(2, 1, a);

  // butBack.createButton("Back", 80, 50, 40, 295, 2, ILI9341_CYAN);
  // butHome.createButton("Home", 80, 50, 200, 295, 2, ILI9341_CYAN);
#elif CASE == 6
#endif
}

void loop()
{
  static unsigned long lastPress = 0;
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
    uint8_t result = mainWindows.checkPress(p);
    if (result != 99)
    {
      Serial.println(a[result]);
    }

#elif CASE == 5
    uint8_t result = shiftedArray.checkPress(p);
    if (result != 99)
    {
      Serial.println(a[result]);
    }
    // else if (butBack.checkPress(p))
    // {
    //   Serial.println(butBack.txt_buf);
    // }
    // else if (butHome.checkPress(p))
    // {
    //   Serial.println(butHome.txt_buf);
    // }
#endif
  }
}
