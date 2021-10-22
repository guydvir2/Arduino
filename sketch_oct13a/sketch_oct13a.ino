#include <TFT_GUI.h>
#include "myIOT_settings.h"
#define SCREEN_ROT 2

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

MessageTFT topTitle;
ButtonTFT homeButton;
buttonArrayTFT<4> mainWindows;
buttonArrayTFT<3> operateWindows;
buttonArrayTFT<8> specificWindows;

const uint16_t TXT_COLOR = ILI9341_BLACK;
const uint16_t FACE_COLOR = ILI9341_GREEN;
const uint16_t BACKGROUND_COLOR = ILI9341_BLACK;

uint8_t menus_id = 0;
uint8_t button_id = 0;
uint8_t last_button_pressed = 0;
const uint8_t mainWindows_id = 10;
const uint8_t specificWindows_id = 20;
const uint8_t operWindows_id = 30;

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
void clearScreen(const uint16_t c)
{
  tft.fillScreen(c);
}
void start_touchScreen()
{
  ts.begin();
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
}

void create_mainWindows()
{
  menus_id = mainWindows_id;
  char *a[] = {"All", "Saloon", "Room", "Specific"};

  mainWindows.scale_y = 72;
  mainWindows.shift_y = 35;
  mainWindows.txt_color = TXT_COLOR;
  mainWindows.face_color = FACE_COLOR;
  mainWindows.border_color = mainWindows.face_color;
  mainWindows.create_array(2, 2, a);
}
void create_specificWindows()
{
  menus_id = specificWindows_id;
  char *specificTitle[] = {"Family", "Parents", "Kids", "Single", "Dual", "Exit", "Laundry", "X"};
  specificWindows.scale_y = 72;
  specificWindows.shift_y = 35;
  specificWindows.txt_color = TXT_COLOR;
  specificWindows.face_color = FACE_COLOR;
  specificWindows.border_color = specificWindows.face_color;
  specificWindows.create_array(4, 2, specificTitle);
}
void create_operWindows()
{
  menus_id = operWindows_id;
  char *operTitle[] = {"Up", "Off", "Down"};
  operateWindows.scale_y = 72;
  operateWindows.shift_y = 35;
  operateWindows.txt_color = TXT_COLOR;
  operateWindows.face_color = FACE_COLOR;
  operateWindows.border_color = operateWindows.face_color;
  operateWindows.create_array(3, 1, operTitle);
}
void buttons_cb(uint8_t i)
{
  char *top = "myHome/Windows";
  char *cmds[] = {"up", "off", "down"};
  char *specif[] = {"familyRoom", "parentsRoom", "kidsRoom", "saloonSingle", "saloonDual", "saloonExit", "Laundry", "X"};

  if (i != 99)
  {
    char fultop[30];
    if (button_id == mainWindows_id)
    {
      iot.pub_noTopic(cmds[i], top);
    }
    else if (button_id == mainWindows_id + 1)
    { // Sallon Windows
      for (uint8_t a = 3; a < 6; a++)
      {
        sprintf(fultop, "%s/%s", top, specif[a]);
        iot.pub_noTopic(cmds[i], fultop);
      }
    }
    else if (button_id == mainWindows_id + 2) // Room Windows
    {
      for (uint8_t a = 0; a < 3; a++)
      {
        sprintf(fultop, "%s/%s", top, specif[a]);
        iot.pub_noTopic(cmds[i], fultop);
      }
    }
    else if (button_id >= specificWindows_id && button_id < specificWindows_id + 7) // Room Windows
    {
      sprintf(fultop, "%s/%s", top, specif[button_id - specificWindows_id]);
      iot.pub_noTopic(cmds[i], fultop);
    }
    else
    {
      yield;
    }
  }
  else
  {
    yield;
  }
}

void windows_button_looper(TS_Point &p)
{
  if (menus_id == mainWindows_id)
  {
    uint8_t i = mainWindows.checkPress(p);
    if (1 != 99)
    {
      if (i != 3)
      {
        button_id = mainWindows_id + i;
        rebuild_screen();
        create_operWindows();
      }
      else
      {
        rebuild_screen();
        create_specificWindows();
      }
    }
  }
  else if (menus_id == specificWindows_id)
  {
    uint8_t i = specificWindows.checkPress(p);
    if (i != 99)
    {
      button_id = specificWindows_id + i;
      rebuild_screen();
      create_operWindows();
    }
  }
  else if (menus_id == operWindows_id)
  {
    uint8_t i = operateWindows.checkPress(p);
    buttons_cb(i);
  }
}
void homeButton_looper(TS_Point &p)
{
  if (homeButton.checkPress(p))
  {
    rebuild_screen();
    create_mainWindows();
  }
}

void create_topTitle()
{
  topTitle.a = tft.width();
  topTitle.b = 30;
  topTitle.xc = tft.width() / 2;
  topTitle.yc = topTitle.b / 2;

  topTitle.txt_size = 1;
  topTitle.roundRect = false;
  topTitle.txt_color = ILI9341_WHITE;
  topTitle.face_color = ILI9341_BLUE;
  topTitle.border_color = topTitle.face_color;
  topTitle.createMSG("<< wait... >>");
}
void update_topTitle(char *msg)
{
  topTitle.createMSG(msg);
}
void check_lan()
{
  char IPadd[16];
  char clk[20];
  static unsigned long lastLoop = 0;
  static bool lastState = false;

  iot.return_clock(clk);
  if (WiFi.isConnected())
  {
    sprintf(IPadd, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  }
}
void create_homeButton()
{
  homeButton.a = tft.width();
  homeButton.b = 40;
  homeButton.xc = tft.width() / 2;
  homeButton.yc = tft.height() - homeButton.b / 2;

  homeButton.txt_size = 2;
  homeButton.roundRect = false;
  homeButton.txt_color = ILI9341_WHITE;
  homeButton.face_color = ILI9341_RED;
  homeButton.border_color = homeButton.face_color;
  homeButton.createButton("<< Home >>");
}
void rebuild_screen()
{
  clearScreen(BACKGROUND_COLOR);
  create_topTitle();
  create_homeButton();
}
void setup()
{
  // put your setup code here, to run once:
  start_touchScreen();
  rebuild_screen();
  create_mainWindows();
  startIOTservices();
  update_topTitle("192.168.3.123");
}

void loop()
{
  // put your main code here, to run repeatedly:
  static unsigned long lastPress_counter = 0;
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    {
      windows_button_looper(p);
      homeButton_looper(p);
      delay(500);
    }
  }
  iot.looper();
}
