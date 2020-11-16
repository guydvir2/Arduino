#include <SPI.h>;
#include <Adafruit_GFX.h>;
#include <Adafruit_ILI9341.h>;
#include <XPT2046_Touchscreen.h>;
 
#define TFT_CS D0  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_DC D8  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TFT_RST -1 //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
#define TS_CS D3   //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
 
//#define TFT_CS 14  //for D32 Pro
//#define TFT_DC 27  //for D32 Pro
//#define TFT_RST 33 //for D32 Pro
//#define TS_CS  12 //for D32 Pro
 
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS);
 
void setup()
{
  Serial.begin(115200);
  ts.begin();
  ts.setRotation(1);
 
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.println("Touch Test");
 
  while (!Serial &&(millis() <= 1000))
    ;
}
 
void loop()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
 
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLUE);
    tft.drawRoundRect(10,10,100,100,5,ILI9341_CYAN);
    tft.fillRoundRect(200,200,100,80,5,ILI9341_DARKGREY);
 
    tft.print("Pressure = ");
    tft.println(p.z);
    tft.print("X = ");
    tft.println(p.x);
    tft.print("Y = ");
    tft.println(p.y);
    Serial.println("BEEP");
  }
}
