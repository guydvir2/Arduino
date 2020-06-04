#include <myDisplay.h>
#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
#define USE_TELEGRAM false
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "emptyCode"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define TELEGRAM_OUT_TOPIC "Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

void startIOTservices()
{
  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.useResetKeeper = USE_RESETKEEPER;
  iot.resetFailNTP = USE_FAILNTP;
  iot.useTelegram = USE_TELEGRAM;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  strcpy(iot.telegramServer, TELEGRAM_OUT_TOPIC);

  iot.start_services(ADD_MQTT_FUNC);
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    // sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help") == 0)
  {
    sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg);
  }
}

char time_str[15];
char date_str[15];

// myOLED OLED(64);
myLCD lcd(16, 2);

void gen_clock()
{
  iot.return_clock(time_str);
  iot.return_date(date_str);
  // OLED.CenterTXT(time_str, "", "", date_str);
  lcd.CenterTXT(time_str, date_str);
}

void setup()
{
  // OLED.start();
  lcd.start();

  // OLED.char_size = 1;
  startIOTservices();
}
void loop()
{
  iot.looper();
  gen_clock();
  delay(100);
}