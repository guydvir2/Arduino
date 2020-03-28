#include <myIOT.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WEMOS_SHT3X.h>
 

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
  static bool readDHTonce = false;
  if (millis() - lastRead_DHT > readInterval)
  {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    lastRead_DHT = millis();
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~ DHT30 ~~~~~~~~~~~~~~~~~~
SHT3X sht30(0x45);
 
void DHT30_loop() {
 
  if(sht30.get()==0){
    Serial.print("Temperature in Celsius : ");
    Serial.println(sht30.cTemp);
    Serial.print("Temperature in Fahrenheit : ");
    Serial.println(sht30.fTemp);
    Serial.print("Relative Humidity : ");
    Serial.println(sht30.humidity);
    Serial.println();
  }
  else
  {
    Serial.println("DHT30_Error!");
  }
  // delay(1000);
 
}

// ~~~~~~~~~~~~~~~~~~~~ LCD ~~~~~~~~~~~~~~~~~~~~
int lcdColumns = 16;
int lcdRows = 2;
byte time_display_clock = 5; //seconds
byte time_display_DHT = 2;   //seconds

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
void startLCD()
{
  lcd.init();
  lcd.backlight();
}
void LCD_DHT()
{
  char t[16];
  char h[16];

  readDHT();
  sprintf(t, "Temp: %.1f%cC", temperature, 223);
  sprintf(h, "Humidity: %.0f%%", humidity);
  lcd.setCursor((int)(16 - strlen(t)) / 2, 0);
  lcd.print(t);
  lcd.setCursor((int)(16 - strlen(h)) / 2, 1);
  lcd.print(h);
  // lcd.clear();
}

void LCD_Clock()
{
  char timeStamp[30];
  char dateStamp[30];

  iot.return_clock(timeStamp);
  iot.return_date(dateStamp);

  lcd.clear();
  lcd.setCursor((int)(16 - strlen(timeStamp)) / 2, 0);
  lcd.print(timeStamp);
  lcd.setCursor((16 - strlen(dateStamp)) / 2, 1);
  lcd.print(dateStamp);
}

void LCD_loop()
{

  static long time_shuffle = 0;
  if (millis() - time_shuffle <= 5000)
  {
    LCD_Clock();
  }
  else if (millis() - time_shuffle <= 7000 && millis() - time_shuffle > 5000)
  {
    LCD_DHT();
  }
  else
  {
    time_shuffle = millis();
  }
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
  DHT30_loop();
  delay(750);
}
