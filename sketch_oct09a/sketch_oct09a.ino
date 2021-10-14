#include <TFT_GUI.h>
#include "myIOT_settings.h"

#define SCREEN_ROT 0
#define BACKGROUND_COLOR ILI9341_BLACK

MessageTFT topBar;
MessageTFT bottomBar;
MessageTFT dataBox;
ButtonTFT butt;
buttonArrayP_TFT mainMenu;

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const uint8_t bar_width = tft.width();
const uint8_t bar_height = 30;
const uint16_t top_bar_color = ILI9341_RED;
const uint16_t bottom_bar_color = ILI9341_BLUE;
const uint16_t text_bar_color = ILI9341_WHITE;

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
void create_main_menu()
{
  char *text[]{"Windows", "Alarm", "Lights", "WiFi"};
  mainMenu.shift_array = 105;
  mainMenu.create_array(100, 100, 10, 10, 2, 2, text, 2, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE);
}
void update_top_bar(char *txt)
{
  topBar.createMSG(txt, bar_width, bar_height, tft.width() / 2, bar_height / 2, 2, 0, top_bar_color, top_bar_color, text_bar_color, false);
}
void update_bottom_bat(char *txt)
{
  topBar.createMSG(txt, bar_width, bar_height, tft.width() / 2, tft.height() - bar_height / 2, 2, 0, bottom_bar_color, bottom_bar_color, text_bar_color, false);
}
void timely_updates()
{
  static unsigned long lastupdate = 0;
  static bool last_connstate = false;
  if (millis() - lastupdate > 10000)
  {
    iot.get_timeStamp();
    update_bottom_bat(iot.timeStamp);
    lastupdate = millis();
  }
  if (WiFi.isConnected() != last_connstate)
  {
    last_connstate = WiFi.status() == WL_CONNECTED;
    if (last_connstate)
    {
      update_top_bar("WiFi Connected");
    }
    else
    {
      update_top_bar("WiFi Fail");
    }
  }
}
void data_msgBox()
{
  // dataBox.createMSG("123223\n dfdfgdfg",)
  tft.println("RTYERTYERT");
  tft.set
}
void setup()
{
  Serial.begin(115200);
  start_touchScreen();
  update_top_bar("booting up...");
  update_bottom_bat("connecting");

  startIOTservices();
  create_main_menu();
}

void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    mainMenu.checkPress(p);
  }
  iot.looper();
  timely_updates();
}
