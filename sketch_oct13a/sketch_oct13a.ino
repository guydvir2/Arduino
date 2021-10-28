#include <TFT_GUI.h>
#include "myIOT_settings.h"
#define SCREEN_ROT 0

XPT2046_Touchscreen ts(TS_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// MessageTFT topTitle;
// MessageTFT noteTFT;
ButtonTFT pressButton;

const uint16_t TXT_COLOR = ILI9341_WHITE;
const uint16_t FACE_COLOR = ILI9341_DARKGREY;
const uint16_t BACKGROUND_COLOR = ILI9341_BLACK;

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

void create_pressButton()
{
  pressButton.a = tft.width() / 2;
  pressButton.b = 130;
  pressButton.xc = tft.width() / 2;
  pressButton.yc = tft.height() / 2;

  pressButton.txt_size = 2;
  pressButton.roundRect = true;
  pressButton.txt_color = TXT_COLOR;
  pressButton.face_color = FACE_COLOR;
  pressButton.border_color = pressButton.face_color;
  pressButton.latchButton = true;
  pressButton.createButton("button");
}

// void create_topTitle()
// {
//   topTitle.a = tft.width();
//   topTitle.b = 30;
//   topTitle.xc = tft.width() / 2;
//   topTitle.yc = topTitle.b / 2;

//   topTitle.txt_size = 2;
//   topTitle.roundRect = false;
//   topTitle.txt_color = ILI9341_WHITE;
//   topTitle.face_color = ILI9341_RED;
//   topTitle.border_color = topTitle.face_color;
//   topTitle.createMSG("<< wait... >>");
// }
// void create_note()
// {
//   noteTFT.a = tft.width();
//   noteTFT.b = 100;
//   noteTFT.xc = tft.width() / 2;
//   noteTFT.yc = noteTFT.b / 2 + 35;

//   noteTFT.txt_size = 2;
//   noteTFT.roundRect = false;
//   noteTFT.txt_color = ILI9341_WHITE;
//   noteTFT.face_color = FACE_COLOR;
//   noteTFT.border_color = FACE_COLOR;

//   char ipadd[16];
//   char uptime_clk[20];
//   char uptime_day[8];
//   char lines[3][30];

//   iot.convert_epoch2clock(millis() / 1000, 0, uptime_clk, uptime_day);
//   check_lan(ipadd);
//   sprintf(lines[0], "IP: %s", ipadd);
//   sprintf(lines[1], "upTime: %s %s", uptime_day, uptime_clk);
//   sprintf(lines[2], "MQTT: %s", iot.deviceTopic);
//   char *noteLines[] = {lines[0], lines[1], lines[2]};
//   noteTFT.createPage(noteLines, 3);
// }
// void clkUpdate(MessageTFT &txtBox)
// {
//   static unsigned long last_clkUpdate = 0;
//   static bool wifi_constate = false;
//   if (millis() - last_clkUpdate > 1000)
//   {
//     char a[20];
//     char b[20];
//     iot.return_clock(a);
//     // check_lan(b);
//     iot.return_date(b);

//     txtBox.updateTXT(a);

//     last_clkUpdate = millis();

//     if (WiFi.isConnected() != wifi_constate)
//     {
//       wifi_constate = WiFi.isConnected();
//       if (wifi_constate == false)
//       {
//         txtBox.face_color = ILI9341_RED;
//         txtBox.border_color = ILI9341_RED;
//         txtBox.createMSG(txtBox.txt_buf);
//       }
//       else
//       {
//         txtBox.face_color = ILI9341_BLUE;
//         txtBox.border_color = ILI9341_BLUE;
//         txtBox.createMSG(txtBox.txt_buf);
//       }
//     }
//   }
// }
// void check_lan(char IPadd[])
// {
//   static unsigned long lastLoop = 0;
//   if (WiFi.isConnected())
//   {
//     sprintf(IPadd, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
//   }
//   else
//   {
//     sprintf(IPadd, "%d.%d.%d.%d", "x", "x", "x", "x");
//   }
// }

void rebuild_screen()
{
  clearScreen(BACKGROUND_COLOR);
  // create_topTitle();
  // create_homeButton();
}

void setup()
{
  // put your setup code here, to run once:
  start_touchScreen();
  rebuild_screen();
  create_pressButton();

  // delay(1000);
  // create_mainWindows();
  // create_mainAlarm();
  // create_keypadAlarm();
  // create_startScreen();
  startIOTservices();
  // create_note();
}
void loop()
{
  // clkUpdate(topTitle);
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    if (pressButton.checkPress(p))
    {
      Serial.print("HI: ");
      Serial.println(pressButton.latchState);
    }
    delay(500);
  }
  iot.looper();
}
