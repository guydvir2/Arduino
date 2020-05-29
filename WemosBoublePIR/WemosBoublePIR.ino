#include <myPIR.h>
#include <myIOT.h>

#define VER "WEMOS_0.1"
#define USE_NOTIFY_TELE true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "DoublePIR"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
#define TELEGRAM_OUT_TOPIC "myHome/Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************
void sendTelegramServer(char *msg, char *tele_server = TELEGRAM_OUT_TOPIC)
{
  if (USE_NOTIFY_TELE)
  {
    char t[200];
    iot.get_timeStamp(now());
    sprintf(t, "[%s][%s]: %s", iot.timeStamp, iot.deviceTopic, msg);
    iot.mqttClient.publish(tele_server, t);
  }
}
// ±±±±±±±±±±±± Sensor PIR ±±±±±±±±±±±±±±±±±
#define Pin_Sensor_0 D1
#define Pin_Sensor_1 D7
#define TIMER 120         // sec in detection
#define LOGIC_DETECTION 5 // duration to logic detection
#define USE_TIMER false
#define RETRIGGER_TIME 15 // secs to next trigger

PIRsensor PIR0(Pin_Sensor_0, "PIR0", LOGIC_DETECTION, HIGH);
PIRsensor PIR1(Pin_Sensor_1, "PIR1", LOGIC_DETECTION, HIGH);

bool inDetection = false;

void notdet()
{
  sendTelegramServer("Detction");
  iot.pub_msg("Detection");
}
void startPIR0()
{
  PIR0.use_serial = USE_SERIAL;
  PIR0.use_timer = USE_TIMER;
  PIR0.trigger_once = false;
  PIR0.timer_duration = TIMER;
  PIR0.ignore_det_interval = RETRIGGER_TIME; // sec untill next detection
  // PIR0.detect_cb(notdet);
  PIR0.start();
}
void startPIR1()
{
  PIR1.use_serial = USE_SERIAL;
  PIR1.use_timer = USE_TIMER;
  PIR1.trigger_once = false;
  PIR1.timer_duration = TIMER;
  PIR1.ignore_det_interval = RETRIGGER_TIME; // sec untill next detection
  PIR1.start();
}
void startPIR()
{
  startPIR0();
  // startPIR1();
}
void PIRlooper()
{
  PIR0.looper();

  // PIR1.looper();
  // if (PIR0.logic_state && PIR1.logic_state && inDetection==false)
  // {
  //   notdet();
  //   inDetection = true;
  // }
  // else if (PIR0.logic_state == false && PIR1.logic_state==false && inDetection)
  // {
  //   inDetection = false;
  // }

  if (PIR0.logic_state && inDetection==false)
  {
    notdet();
    inDetection = true;
  }
  else if (PIR0.logic_state == false && inDetection)
  {
    inDetection = false;
  }
}

// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

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

void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];

  if (strcmp(incoming_msg, "status") == 0)
  {
    // sprintf(msg, "Status: Device is [%s]", PIR1.swState ? "On" : "Off");
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
    sprintf(msg, "Help: Commands #2 - []");
    iot.pub_msg(msg);
  }
}

void setup()
{
  startPIR();
  startIOTservices();
  sendTelegramServer("start Double PIR");
}

void loop()
{
  iot.looper();
  if (millis() > 45000)
  {
    PIRlooper();
  }

  delay(100);
}