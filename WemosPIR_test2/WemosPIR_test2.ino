#include <Arduino.h>
#include <myIOT.h>
#include <myPIR.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

FVars endTimeOUT_flash("endTO");
FVars bootClock_flash("boot");

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~              // belong to myIOT
#define DEVICE_TOPIC "test"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "extLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Hardware ~~~~~~~~~~~~
#define relayPin D3
#define PIR_PIN D7
#define IS_ON HIGH

bool relayState = false;

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"
#define USE_NOTIFY_TELE true
#define TIMER_DURATION 30 // minutes
#define TO_NICK "Timer"
byte dailtTO_start[] = {22, 39, 30};
byte dailtTO_stop[] = {22, 40, 0};

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true      // Serial Monitor
#define USE_WDT true         // watchDog resets
#define USE_OTA true         // OTA updates
#define USE_RESETKEEPER true // detect quick reboot and real reboots
#define USE_FAILNTP true     // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT ~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~~~~~~~~ Switch Commands ~~~~~~~

void turnoff()
{
  digitalWrite(relayPin, !IS_ON);
}
void turnon()
{
  digitalWrite(relayPin, IS_ON);
}

void switchRelay(bool state, char *activator = TO_NICK)
{

  if (state != relayState)
  {
    relayState = state;
    if (state == 1)
    {
      turnon();
    }
    else
    {
      turnoff();
    }

    char msg[100];
    char stat[10];

    sprintf(stat, "%s", state ? "On" : "Off");
    sprintf(msg, "[%s] Switched [%s]", activator, stat);
    iot.pub_msg(msg);
    iot.pub_state(stat);
  }
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

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
#if USE_NOTIFY_TELE

char *Telegram_Nick = DEVICE_TOPIC; //"iotTest";
int time_check_messages = 1;        //sec

myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
  String command_set[] = {"whois_online", "status", "reset", "help", "off", "on", "timeout", "whoami"};
  byte num_commands = sizeof(command_set) / sizeof(command_set[0]);
  String comp_command[num_commands];
  char prefix[100], prefix2[100];
  char t1[50], t2[50];

  sprintf(snd_msg, ""); // when not meeting any conditions, has to be empty

  from.toCharArray(t1, from.length() + 1);
  in_msg.toCharArray(t2, in_msg.length() + 1);

  sprintf(prefix, "/%s_", Telegram_Nick);
  sprintf(prefix2, "from user: %s\ndevice replies: %s\ncommand: %s\n~~~~~~~~~~~~~~~~~~~~\n ", t1, Telegram_Nick, t2);

  for (int i = 0; i < num_commands; i++)
  {
    comp_command[i] = prefix;
    comp_command[i] += command_set[i];
  }

  if (in_msg == "/whois_online")
  {
    sprintf(snd_msg, "%s%s", prefix2, Telegram_Nick);
  }
  else if (in_msg == comp_command[1])
  {
    giveStatus(t1);
    sprintf(snd_msg, "%s%s", prefix2, t1);
  } // status
  else if (in_msg == comp_command[2])
  {
    sprintf(snd_msg, "%s", prefix2);
    iot.sendReset("Telegram");
  } // reset
  else if (in_msg == comp_command[3])
  {
    char t[50];
    sprintf(snd_msg, "%sCommands Available:\n", prefix2, Telegram_Nick);
    for (int i = 0; i < num_commands; i++)
    {
      command_set[i].toCharArray(t, 30);
      sprintf(t1, "%s\n", t);
      strcat(snd_msg, t1);
    }

  } // all_commands

  else if (in_msg == comp_command[4])
  {
    // all_off("Telegram");
    sprintf(snd_msg, "%sAll-Off signal was sent", prefix2);
  } // off
  else if (in_msg == comp_command[5])
  {
  }
  else if (in_msg == comp_command[6])
  {
  } // timeout
  else if (in_msg == comp_command[7])
  {
    sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, DEVICE_TOPIC);
  } // whoami
}
#endif

// ~~~~~~ Time Alarm ~~~~~~~
AlarmId timer_id;
AlarmId daily_id;

void Alarm_clockupdate()
{
  if (iot.NTP_OK)
  {
    time_t t = now();
    setTime(hour(t), minute(t), second(t), day(t), month(t), year(t));
    bootClock_flash.setValue(t);
  }
}

void updateflash_endTime(int dur, long start = 0)
{
  if (start == 0)
  {
    start = now();
  }
  unsigned long endto = (long)(start + dur);

  long t;
  endTimeOUT_flash.getValue(t);
  if (t != endto)
  {
    endTimeOUT_flash.setValue((long)endto);
  }
}

void startEveningAlarm()
{
  switchRelay(1, "Daily-Timer");

  unsigned long timedelta = 0;
  int h = dailtTO_stop[0] - dailtTO_start[0];
  int m = dailtTO_stop[1] - dailtTO_start[1];
  int s = dailtTO_stop[2] - dailtTO_start[2];

  if (h < 0)
  {
    h = h + 24;
  }
  if (m < 0)
  {
    m = m + 60;
    h--;
  }
  if (s < 0)
  {
    s = s + 60;
    m--;
  }

  timedelta = h * 3600 + m * 60 + s;
  updateflash_endTime(timedelta);
}
void endEveningAlarm()
{
  switchRelay(0, "Daily-Timer");
}
void startDailyTimer()
{
  Alarm.alarmRepeat(dailtTO_start[0], dailtTO_start[1], dailtTO_start[2], startEveningAlarm);
  Alarm.alarmRepeat(dailtTO_stop[0], dailtTO_stop[1], dailtTO_stop[2], endEveningAlarm);
}
void end_timer()
{
  switchRelay(0);
  endTimeOUT_flash.setValue(0);
}

void start_timer(int dur = 30, char *activ = TO_NICK)
{
  switchRelay(1, activ);
  Alarm.timerOnce(dur, end_timer);
  updateflash_endTime(dur);
}
void restore_timer()
{
  long savedVal;
  int restoreVal;

  endTimeOUT_flash.getValue(savedVal);
  Serial.println(savedVal);
  if (savedVal != 0 && now() < savedVal)
  {
    restoreVal = (int)(savedVal - now());
    start_timer(restoreVal);
    Serial.println("RESTORING");
  }
}
int remain_timer()
{
  long t;
  endTimeOUT_flash.getValue(t);

  if (t > 0 && t > now() && relayState == 1)
  {
    return t - now();
  }
  else
  {
    return 0;
  }
}

void disable_dailyTimer()
{
}

// ~~~~~~~~~~~~~~~~~ PIR Sensor ~~~~~~~~~~~~~~
#define PIN_TO_SENSOR_1 PIR_PIN
#define TIMER 120          // sec in detection
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
  sprintf(str2, "PIRsensor Detection [#%d]", counter);
  sprintf(str1, "[%s %s] [%s] %s", dateStamp, timeStamp, device, str2);
  iot.pub_msg(str2);

#if USE_NOTIFY_TELE
  teleNotify.send_msg(str1);
#endif
}

void flicker_on_detection()
{
  for (int a = 0; a < 10; a++)
  {
    digitalWrite(relayPin, !relayState);
    relayState = !relayState;
    delay(200);
  }
}

void detect_callback()
{
  time_t detect_time = now();
  bool time_to_on = false;

  if (relayState == !IS_ON)
  {
    flicker_on_detection();
  }
  // if (hour(detect_time) >= 18 && hour(detect_time) <= 7)
  // {
  //   time_to_on = true;
  // }
  if (relayState == !IS_ON)
  {
    switchRelay(1, "PIRsensor");
  }
  else
  {
    flicker_on_detection();
  }
  notifyDetection(sensor0.sensNick, sensor0.detCounts);
}
void end_detection_callback()
{
  switchRelay(0, "PIRsensor");
}

void startPIR()
{
  sensor0.use_serial = false;
  sensor0.use_timer = true;
  sensor0.trigger_once = false;
  sensor0.timer_duration = TIMER;
  sensor0.ignore_det_interval = 60; // sec untill next detection
  sensor0.detect_cb(detect_callback);
  sensor0.end_detect_cb(end_detection_callback);
  sensor0.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
    start_timer();
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "off") == 0)
  {
    sprintf(msg_MQTT, "Command: Turn [Off]");
    end_timer();
    iot.pub_msg(msg_MQTT);
  }
}
void giveStatus(char *outputmsg)
{
  // sprintf(outputmsg, "Status: Detections [#%d], Notify [%s], Active [%s]", sensor0.detCounts, no_notify ? "No" : "Yes", sensor0.stop_sensor ? "Yes" : "No");
}

void startGPIOs()
{
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);
}

void checkRebootState()
{
  static bool checkreboot = true;
  int rebstate = iot.mqtt_detect_reset;

  if (rebstate != 2 && checkreboot)
  {
    checkreboot = false;
    if (rebstate == 0)
    {
      Serial.println("Just an Ordinary Boot");
    }
    else
    {
      restore_timer();
    }
  }
}

void setup()
{
  startGPIOs();
  startIOTservices();
  startPIR();
  delay(15000);

  // Alarm_clockupdate();
  // startDailyTimer();

#if USE_NOTIFY_TELE
  teleNotify.begin(telecmds);
#endif
}

void loop()
{
  iot.looper();
  sensor0.looper();
  delay(100);
#if USE_NOTIFY_TELE
  teleNotify.looper();
#endif

// #if USE_RESETKEEPER
//   checkRebootState();
// #endif

  // Alarm.delay(100);
}
