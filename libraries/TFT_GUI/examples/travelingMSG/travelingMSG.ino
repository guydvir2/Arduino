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
  const uint8_t txt_size = 1;
  const uint8_t MSG_size_y = 75;
  const uint8_t MSG_size_x = 100;

  MSGwindow.screen_rotation = SCREEN_ROT;
  MSGwindow.txt_size = txt_size;
  MSGwindow.a = MSG_size_x;
  MSGwindow.b = MSG_size_y;
  MSGwindow.xc = tft.width() / 2;
  MSGwindow.yc = MSGwindow.b / 2;
  MSGwindow.border_thickness = 2;
  MSGwindow.roundRect = true;
  MSGwindow.createMSG("Hello World");
}
void MSG_travel()
{
  const int X_MIN = MSGwindow.a / 2;
  const int X_MAX = tft.width() - MSGwindow.a / 2;
  const int Y_MIN = MSGwindow.b / 2;
  const int Y_MAX = tft.height() - MSGwindow.b / 2;

  Serial.print("X_range: ");
  Serial.print(X_MIN);
  Serial.print("-");
  Serial.println(X_MAX);
  Serial.print("Y_range: ");
  Serial.print(Y_MIN);
  Serial.print("-");
  Serial.println(Y_MAX);

  long XC = random(X_MIN, X_MAX);
  long YC = random(Y_MIN, Y_MAX);

  MSGwindow.xc = XC;
  MSGwindow.yc = YC;

  Serial.print(MSGwindow.xc);
  Serial.print(",");
  Serial.println(MSGwindow.yc);
  tft.fillScreen(ILI9341_BLUE);

  MSGwindow.createMSG(MSGwindow.txt_buf);
  delay(2000);
}
void setup()
{
  Serial.begin(115200);
  start_GUI();
  GUI_properties();
  create_MSG();
}

void loop()
{
  // put your main code here, to run repeatedly:
  MSG_travel();
}
