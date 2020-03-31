#include <myIOT.h>
#include <Arduino.h>
#include <myPIR.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~              // belong to myIOT
#define DEVICE_TOPIC "pirtest2"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT ~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~ MQTT Commands ~~~~~
void addiotnalMQTT(char *income_msg)
{
  char msg_MQTT[150];
  char msg2[20];

  if (strcmp(income_msg, "status") == 0)
  {
    giveStatus(msg_MQTT);
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "ver") == 0)
  {
    sprintf(msg_MQTT, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "help") == 0)
  {
    sprintf(msg_MQTT, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "on") == 0)
  {
    sprintf(msg_MQTT, "Command: Turn [On]");
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "off") == 0)
  {
    sprintf(msg_MQTT, "Command: Turn [Off]");
    iot.pub_msg(msg_MQTT);
  }
}
void giveStatus(char *outputmsg)
{
  // sprintf(outputmsg, "Status: Detections [#%d], Notify [%s], Active [%s]", sensor0.detCounts, no_notify ? "No" : "Yes", sensor0.stop_sensor ? "Yes" : "No");
}

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
// ~~~~~~ Time Alarm ~~~~~~~
#include <TimeLib.h>
#include <TimeAlarms.h>

AlarmId id;
FVars endTimeOUT_flash;

void Alarm_clockupdate()
{
  if (iot.NTP_OK)
  { // Store the current time in time
    time_t t = now();
    setTime(hour(t), minute(t), second(t), day(t), month(t), year(t)); //8,29,0,1,1,11); // set time to Saturday 8:29:00am Jan 1 2011
    endTimeOUT_flash.setValue(t);
    long getval;
    endTimeOUT_flash.getValue(getval);
  }
}

// ~~~~~~~~~~~~~~~~~ PIR Sensor ~~~~~~~~~~~~~~
#define PIN_TO_SENSOR_1 D7
#define TIMER 10           // sec in detection
#define LOGIC_DETECTION 10 // duration to logic detection

PIRsensor sensor0(PIN_TO_SENSOR_1, DEVICE_TOPIC, LOGIC_DETECTION, HIGH);

void notifyDetection(char *device = "empTy", int counter = 0)
{
  char str1[40];
  char str2[40];
  char timeStamp[16];
  char dateStamp[16];

  iot.return_clock(timeStamp);
  iot.return_date(dateStamp);
  sprintf(str2, "[%s] Detection [#%d]", device, counter);
  sprintf(str1, "[%s %s] %s", dateStamp, timeStamp, str2);
  iot.pub_msg(str2);
}

void detect_callback()
{
  Serial.println("DETECT");
  notifyDetection(sensor0.sensNick, sensor0.detCounts);
  Alarm.timerOnce(10, temp_not);
  // endTimeOut();
}

void startSensors()
{
  sensor0.use_serial = true;
  sensor0.use_timer = false;
  sensor0.trigger_once = false;
  sensor0.timer_duration = TIMER;
  sensor0.ignore_det_interval = 25; // sec untill next detection
  sensor0.detect_cb(detect_callback);
  // sensor0.run_enddet_func(end_detection_callback);
  sensor0.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void temp_not()
{
  notifyDetection("end", 555);
}

void startGPIOs()
{
  pinMode(PIN_TO_SENSOR_1, INPUT);
  pinMode(D3, OUTPUT);
}

void end_detection_callback()
{
  Serial.println("END_DETECT");
}

// void endTimeOut()
// {
//   Alarm.timerOnce(10, temp_not);
// }

void setup()
{
  startGPIOs();
  startIOTservices();
  Alarm_clockupdate();
  startSensors();
  // Alarm.alarmRepeat(21,42,30, temp_not);
}

void loop()
{
  iot.looper();
  sensor0.looper();

  Alarm.delay(100);
}
