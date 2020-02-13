//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
int detCounter = 0;
bool lastState = false;
bool detection = HIGH;

void printlin1(char *txt) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(txt);
}
void printlin2(int i) {
  lcd.setCursor(0, 1);
  char t [20];
  sprintf(t, "detections:#%d", i);
  lcd.print(t);

}

void setup()
{
  pinMode(2, INPUT);
  lcd.init();                      // initialize the lcd
  lcd.backlight();
}

void loop()
{
  if (digitalRead(2) != lastState) {
    lastState = !lastState;
    if ( lastState == detection ) {
      printlin1("Detection");
      detCounter++;
    }
    else {
      printlin1("no Detect");
    }
    printlin2(detCounter);
  }
  delay(200);
}
