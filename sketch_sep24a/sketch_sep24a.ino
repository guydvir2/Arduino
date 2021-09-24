#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1

#define SCREEN_ROT 3
int TFT_W = 0;
int TFT_H = 0;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
MessageTFT MSGwindow(tft);

void start_GUI()
{
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
void create_MSG()
{
  const uint8_t txt_size = 3;
  const uint8_t MSG_size_x = 230;
  const uint8_t MSG_size_y = 75;
  const uint8_t x_margin = 20;
  const uint8_t y_margin = 20;

  MSGwindow.init();
  MSGwindow.screen_rotation = SCREEN_ROT;
  strcpy(MSGwindow.txt_buf, "Hello World");
  MSGwindow.txt_size = txt_size;
  MSGwindow.a = MSG_size_x;
  MSGwindow.b = MSG_size_y;
  MSGwindow.xc = TFT_W / 2;
  MSGwindow.yc = TFT_H / 2;
  MSGwindow.border_thickness = 5;
  MSGwindow.drawMSG();
}
void setup()
{
  start_GUI();
  GUI_properties();
  create_MSG();
}

void loop()
{
  int pos = 0;
  while (MSGwindow.yc + MSGwindow.b/2 <= TFT_H ){
    MSGwindow.yc += 10;
    tft.fillScreen(ILI9341_BLUE);
    MSGwindow.drawMSG();
    delay(1000);
  }
  while (MSGwindow.yc-MSGwindow.b / 2 >=0)
  {
    MSGwindow.yc -= 10;
    tft.fillScreen(ILI9341_BLUE);
    MSGwindow.drawMSG();
    delay(1000);
  }
  // put your main code here, to run repeatedly:
}
