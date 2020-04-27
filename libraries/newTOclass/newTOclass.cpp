#include <Arduino.h>
#include <myIOT.h>

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

#include <TimeLib.h>
#include <TimeAlarms.h>
#include <myIOT.h>

// ~~~~~~~~~~~ Switch Commands ~~~~~~~
class CronJobs
{

private:
  FVars endTimeOUT_flash("endTO");
  FVars bootClock_flash("boot");
  time_t _clock_sync;

private:
  AlarmId timer_id;
  AlarmId daily_id;

  void Alarm_clockupdate(time_t t)
  {
    // time_t t = now();
    setTime(hour(t), minute(t), second(t), day(t), month(t), year(t));
    // bootClock_flash.setValue(t);
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
  void calc_end_Alarm()
  {
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
  void beginAlarm()
  {
    // switchRelay(1, "Daily-Timer");
    calc_end_Alarm();
  }
  void endAlarm()
  {
    // switchRelay(0, "Daily-Timer");
  }
  void startAlarm_services()
  {
    Alarm.alarmRepeat(dailtTO_start[0], dailtTO_start[1], dailtTO_start[2], beginAlarm);
    Alarm.alarmRepeat(dailtTO_stop[0], dailtTO_stop[1], dailtTO_stop[2], endAlarm);
  }

public:
  CronJobs(time_t clock_sync)
  {
    _clock_sync = clock_sync;
  }

  // ~~~~~~ Timer Tasks -- Happen once -- ~~~~~~
  void end_timer()
  {
    // switchRelay(0);
    endTimeOUT_flash.setValue(0);
  }

  void start_timer(int dur = 30, char *activ = TO_NICK)
  {
    // switchRelay(1, activ);
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

  void looper()
  {
    Alarm.delay(100);
  }
};

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
  // pinMode(relayPin, OUTPUT);
  // digitalWrite(relayPin, relayState);
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
      // restore_timer();
    }
  }
}

void setup()
{
  startGPIOs();
  startIOTservices();

  // Alarm_clockupdate();
  // startAlarm_services();

  void loop()
  {
    iot.looper();
    delay(100);

    // #if USE_RESETKEEPER
    //   checkRebootState();
    // #endif

    // Alarm.delay(100);
  }
