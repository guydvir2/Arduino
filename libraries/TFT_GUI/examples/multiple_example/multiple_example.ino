#include <TFT_GUI.h>

#define SCREEN_ROT 0

#define CASE 6
#if CASE == 1 /* Simple MsgBox - No Touch */
MessageTFT MsgBox;

#elif CASE == 2 /* Simple Button - incl Touch */
ButtonTFT butt;

#elif CASE == 3 /* 12 button preDefined Keypad - pass code after pressing "*" */
keypadTFT keypad;

#elif CASE == 4 /* Button Array - equally spaced buttons */
char *a[] = {"All Windows", "Saloon", "Room", "Specific"};
char *operTitle[] = {"Up", "Off", "Down"};
char *specificTitle[] = {"Family", "Parents", "Kids", "Single", "Dual", "Exit", "Laundry", "X"};
buttonArrayTFT mainWindows;
buttonArrayTFT operateWindows;
buttonArrayTFT specificWindows;
uint8_t stage_level = 0;

#elif CASE == 5 /* Shifted Array and 2 single Buttons */
char *a[] = {"F1", "F2"};
buttonArrayTFT shiftedArray;
// ButtonTFT butBack;
// ButtonTFT butHome;
#elif CASE==6
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
  MsgBox.a = 200;
  MsgBox.b = 60;
  MsgBox.xc = tft.width() / 2;
  MsgBox.yc = tft.height() / 2;
  MsgBox.txt_size = 1;
  MsgBox.createMSG("msgBox");

#elif CASE == 2
  butt.a = 200;
  butt.b = 60;
  butt.xc = tft.width() / 2;
  butt.yc = tft.height() - butt.b / 2;
  butt.txt_size = 3;
  butt.face_color = ILI9341_CASET;
  butt.txt_color = ILI9341_WHITE;
  butt.border_color = ILI9341_MAGENTA;
  butt.roundRect = false;
  butt.createButton("Press!");

#elif CASE == 3
  keypad.scale_f = 95; /* % of max screen size */
  // keypad.shift_x = 50; /* shift x axis, regarding to top left corner. commenting out will center keypad */
  // keypad.shift_y = 100;
  keypad.txt_size = 2;
  keypad.txt_color = ILI9341_WHITE;
  keypad.face_color = ILI9341_CASET;
  keypad.border_color = ILI9341_LIGHTGREY;
  keypad.roundRect = false;
  keypad.create_keypad();

#elif CASE == 4
  // mainWindows.shrink_shift = 240;
  // mainWindows.shift_y = 100; /* Shift witout resize */
  // mainWindows.shift_x = 100; /* Shift witout resize */
  mainWindows.scale_f = 100;
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
  shiftedArray.dx = 50;
  shiftedArray.dy = 10;
  shiftedArray.shift_y = 0;
  shiftedArray.txt_size = 2;
  shiftedArray.face_color = ILI9341_GREENYELLOW;
  shiftedArray.txt_color = ILI9341_WHITE;
  shiftedArray.border_color = ILI9341_GREENYELLOW;
  shiftedArray.roundRect = false;
  // shiftedArray.shrink_shift = -60;
  shiftedArray.create_array(1,2, a);

  // butBack.createButton("Back", 80, 50, 40, 295, 2, ILI9341_CYAN);
  // butHome.createButton("Home", 80, 50, 200, 295, 2, ILI9341_CYAN);
#elif CASE==6
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
    lastPress = millis();
    if (stage_level == 0) /* mainMenu*/
    {
      uint8_t result = mainWindows.checkPress(p);
      if (result != 99)
      {
        if (result != 3)
        {
          stage_level = 50 + result;
          clearScreen();
          operateWindows.create_array(3, 1, operTitle);
        }
        else
        {
          stage_level = 1;
          clearScreen();
          specificWindows.create_array(4, 2, specificTitle);
        }
      }
    }
    else if (stage_level == 1) /* Specific */
    {
      uint8_t result = specificWindows.checkPress(p);
      if (result != 99)
      {
        stage_level = 60 + result;
        clearScreen();
        operateWindows.create_array(3, 1, operTitle);
      }
    }
    else
    {
      uint8_t result = operateWindows.checkPress(p);
      if (result != 99)
      {
        Serial.println(stage_level);
        Serial.println(operTitle[result]);
        clearScreen();
        stage_level = 0;
        mainWindows.create_array(4, 1, a);
      }
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
