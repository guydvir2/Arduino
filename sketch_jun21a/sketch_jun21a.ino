#include <myIOT.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.1"
#define USE_TELEGRAM true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "RADARsense"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "test"
#define TELEGRAM_TOPIC "myHome/Telegram"
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
  iot.useextTopic = USE_TELEGRAM;
  iot.resetFailNTP = USE_FAILNTP;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  strcpy(iot.extTopic, TELEGRAM_TOPIC);
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

const int sensorPin = D1;
bool lastState = false;

void setup()
{
  startIOTservices();
  pinMode(sensorPin, INPUT);
}
void loop()
{
  iot.looper();
  if (millis() > 30000)
  {
    if (digitalRead(sensorPin) && lastState == false)
    {
      lastState = true;
      // Serial.println("Detection");
      iot.pub_msg("Radar_Detect");
      iot.pub_ext("Radar_Detect");
    }
    else if (!digitalRead(D1) && lastState)
    {
      lastState = false;
      // Serial.println("END_Detection");
      iot.pub_msg("END - Radar_Detect");
    }
  }
  delay(100);
}
