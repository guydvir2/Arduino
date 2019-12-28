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

void setup()
{
  startIOTservices();
  
  lcd.init();
  lcd.backlight();
}

void loop()
{
  iot.looper();
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print("Hello, World!");
  delay(1000);
  // clears the display to print new message
  lcd.clear();
  // set cursor to first column, second row
  lcd.setCursor(0, 1);
  lcd.print("Hello, World!");
  delay(1000);
  lcd.clear();

  delay(100);
}
