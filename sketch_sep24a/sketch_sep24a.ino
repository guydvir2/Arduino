#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TFT_GUI.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1

#define SCREEN_ROT 3

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
MessageTFT MSGwindow(tft);

void start_GUI()
{
  tft.begin();
  tft.setRotation(SCREEN_ROT); /* 0-3 90 deg each */
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
  tft.print(tft.width());
  tft.print("X");
  tft.print(tft.height());
}
void create_MSG()
{
  const uint8_t txt_size = 3;
  const uint8_t MSG_size_y = 75;
  const uint8_t MSG_size_x = 230;

  MSGwindow.screen_rotation = SCREEN_ROT;
  MSGwindow.txt_size = txt_size;
  MSGwindow.a = MSG_size_x;
  MSGwindow.b = MSG_size_y;
  MSGwindow.xc = tft.width() / 2;
  MSGwindow.yc = tft.height() / 2;
  MSGwindow.border_thickness = 5;
  MSGwindow.roundRect = true;
  MSGwindow.text("Hello World");
  MSGwindow.drawMSG();
}
void MSG_travel()
{
  const int X_MIN = MSGwindow.xc - MSGwindow.a / 2;
  const int X_MAX = MSGwindow.xc + MSGwindow.a / 2;
  const int Y_MIN = MSGwindow.yc - MSGwindow.b / 2;
  const int Y_MAX = MSGwindow.yc + MSGwindow.b / 2;

  randomSeed(analogRead(0));
  long XC = random(X_MIN, X_MAX);
  long YC = random(Y_MIN, Y_MAX);

  MSGwindow.xc = (int)XC;
  MSGwindow.yc = (int)YC;
  create_MSG();
  delay(2000);
}
void setup()
{
  start_GUI();
  GUI_properties();
  create_MSG();
}

void loop()
{
  // put your main code here, to run repeatedly:
  MSG_travel();
}
