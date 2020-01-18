#include <myIOT.h>
#include <LiquidCrystal_I2C.h>

#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"
#define USE_NOTIFY_TELE false

// ********** myIOT Class ***********

//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false
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

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

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

// ~~~~~~~~~~~ BMP ~~~~~~~~~~~
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#define DHTPIN D4     // Digital pin connected to the DHT sens
#define DHTTYPE DHT11 // DHT 11

Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);

void startBMP()
{
  bmp.begin();
}
void startDHT()
{
  dht.begin();
}

void setup()
{
  startIOTservices();

  lcd.init();
  lcd.backlight();

  startBMP();
  startDHT();
}

void loop()
{
  iot.looper();
  // set cursor to first column, first row
  // lcd.setCursor(0, 0);
  // // print message
  // lcd.print("Hello, World!");
  // delay(1000);
  // // clears the display to print new message
  // lcd.clear();
  // // set cursor to first column, second row
  // lcd.setCursor(0, 1);
  // lcd.print("Hello, World!");
  // delay(1000);
  // lcd.clear();

  
  float h_temp = dht.readHumidity();
  float t_temp = dht.readTemperature();
  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(bmp.readTemperature());
  lcd.print(char(223));
  lcd.print("C");

  lcd.print(" / ");
  lcd.print(t_temp);
  lcd.print(char(223));
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print(bmp.readAltitude(101500));
  delay(1000);
  

  delay(10);
}
