#include <myIOT.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"
#define USE_NOTIFY_TELE false

// ********** myIOT Class ***********

//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true
#define USE_WDT true
#define USE_OTA true
#define USE_RESETKEEPER false
#define USE_FAILNTP true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "LCD"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "TESTS"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~~~~~~~~ DHT ~~~~~~~~~~~
#include "DHT.h"
#define DHTPIN D4
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
float humidity;
float temperature;
long lastRead_DHT = 0;
long readInterval = 10000;
DHT dht(DHTPIN, DHTTYPE);

void startDHT()
{
  dht.begin();
}

void readDHT()
{
  if (millis() - lastRead_DHT > readInterval)
  {

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    lastRead_DHT = millis();
    delay(2000);
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~ LCD ~~~~~~~~~~~~~~~~~~~~
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
void startLCD()
{
  lcd.init();
  lcd.backlight();
}
void LCD_DHT()
{
  readDHT();
  char t[16];
  char h[16];
  sprintf(t, "%.1f%cC %.0f%%", temperature,223, humidity);
  Serial.println(sizeof(t)/(sizeof(t[0])));
  // lcd.print(temperature);
  // lcd.print(char(223));
  // lcd.print("C  ");
  lcd.setCursor(0, 0);
  lcd.print(t);
  // lcd.print(humidity);
  // lcd.print("%");
}

void LCD_Clock()
{
  char timeStamp[30];
  char dateStamp[30];

  iot.return_clock(timeStamp);
  iot.return_date(dateStamp);

  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dateStamp);
  lcd.setCursor(0, 1);
  lcd.print(timeStamp);
}

void LCD_loop()
{
  LCD_DHT();
  // LCD_Clock();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void startIOTservices()
{
  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.useResetKeeper = USE_RESETKEEPER;
  iot.resetFailNTP = USE_FAILNTP;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char incoming_msg[50])
{
  char state[5];
  char state2[5];
  char msg[100];
  char msg2[100];

  if (strcmp(incoming_msg, "status") == 0)
  {
    iot.pub_msg("empty_Status");
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d],ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
    iot.pub_msg(msg);
  }
}

void setup()
{
  startIOTservices();
  startLCD();
  startDHT();
}

void loop()
{
  iot.looper();
  LCD_loop();
  delay(300);
}
