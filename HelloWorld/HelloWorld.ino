#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
int detCounter = 0;
int sensorPin = D1;
int buzPin = 3;

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
  pinMode(sensorPin, INPUT);
  pinMode(buzPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
}

void loop()
{
  // if (digitalRead(sensorPin) != lastState) {
  //   lastState = !lastState;
  //   if ( lastState == detection ) {
  //     printlin1("Detection");
  //     detCounter++;
  //     digitalWrite(buzPin,HIGH);
  //     delay(50);
  //     digitalWrite(buzPin, LOW);
  //   }
  //   else {
  //     printlin1("no Detect");
  //   }
  //   printlin2(detCounter);
  // }

  Serial.println(digitalRead(sensorPin));
      delay(100);
}
