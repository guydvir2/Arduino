#include <myIOT.h>
#include <Arduino.h>
#include <myPIR.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~              // belong to myIOT
#define DEVICE_TOPIC "parentsBedLEDs"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "intLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"
#define USE_NOTIFY_TELE true
#define ON_TIME 180 // minutes

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

const int logSize = 8;
bool no_notify = false;
char *initword = "EMPTYCELL";
char detectionLog[logSize][50];
long on_clock = 0;

// ~~~~~~~~~~~~~~~~~ PIR Sensor ~~~~~~~~~~~~~~
#define PIN_TO_SENSOR_1 D1
#define TIMER 10           // sec in detection
#define LOGIC_DETECTION 10 // duration to logic detection

PIRsensor sensor0(PIN_TO_SENSOR_1, DEVICE_TOPIC, LOGIC_DETECTION, HIGH);

void startSensors()
{
  sensor0.use_serial = true;
  sensor0.use_timer = true;
  sensor0.trigger_once = false;
  sensor0.timer_duration = TIMER;
  sensor0.ignore_det_interval = 5;
  sensor0.run_func(detect_callback);
  sensor0.run_enddet_func(end_detection_callback);
  sensor0.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
    turnleds_on();
    sprintf(msg_MQTT, "Command: Turn [On]");
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "off") == 0)
  {
    turnleds_off();
    sprintf(msg_MQTT, "Command: Turn [Off]");
    iot.pub_msg(msg_MQTT);
  }
}
void giveStatus(char *outputmsg)
{
  sprintf(outputmsg, "Status: Detections [#%d], Notify [%s], Active [%s]", sensor0.detCounts, no_notify ? "No" : "Yes", sensor0.stop_sensor ? "Yes" : "No");
}

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
#if USE_NOTIFY_TELE
char *Telegram_Nick = DEVICE_TOPIC;
int time_check_messages = 1; //sec
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
// ~~~~~~~~~~~ Telegram Notify ~~~~~~~

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
  String command_set[] = {"whois_online", "status", "reset", "whoami", "help",
                          "stop_sensor", "restore_sensor", "stop_notify", "restore_notify", "det_log", "remain"};
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
    sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, DEVICE_TOPIC);
  } // whoami
  else if (in_msg == comp_command[4])
  {
    char t[50];
    sprintf(snd_msg, "%sCommands Available:\n", prefix2, Telegram_Nick);
    for (int i = 0; i < num_commands; i++)
    {
      command_set[i].toCharArray(t, 30);
      sprintf(t1, "%s\n", t);
      strcat(snd_msg, t1);
    }
  }
  else if (in_msg == comp_command[5])
  {
    sprintf(snd_msg, "%s~ Sensor stopped", prefix2);
    sensor0.stop_sensor = true;
  }
  else if (in_msg == comp_command[6])
  {
    sprintf(snd_msg, "%s~ Sensor restored", prefix2);
    sensor0.stop_sensor = false;
  }
  else if (in_msg == comp_command[7])
  {
    sprintf(snd_msg, "%s~ Notifications stopped", prefix2);
    no_notify = true;
  }
  else if (in_msg == comp_command[8])
  {
    sprintf(snd_msg, "%s~ Notifications restored", prefix2);
    no_notify = false;
  }
  else if (in_msg == comp_command[9])
  {
    sprintf(snd_msg, "%s~ Detection log:", prefix2);
    char t[20];
    for (int a = 0; a < logSize; a++)
    {
      if (strcmp(detectionLog[a], initword) != 0)
      {
        sprintf(t, "\n%s", detectionLog[a]);
        strcat(snd_msg, t);
      }
    }
  }
  else if (in_msg == comp_command[10])
  {
    int total_sec = (int)sensor0.timeLeft;
    int total_hrs = (int)(total_sec / 3600) > 0 ? (int)(total_sec / 3600) : 0;
    int total_mins = (int)((total_sec - total_hrs * 3600) / 60) > 0 ? (int)((total_sec - total_hrs * 3600) / 60) : 0;
    int total_secs = total_sec - total_mins * 60 - total_hrs * 3600;
    sprintf(snd_msg, "%s~ TimeOut remain: %02d:%02d:%02d", prefix2, total_hrs, total_mins, total_secs);
  }
}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

void startGPIOs()
{
  pinMode(PIN_TO_SENSOR_1, INPUT);
  pinMode(D3, OUTPUT);
}

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

  update_mag(str1);
  iot.pub_msg(str2);

#if USE_NOTIFY_TELE
  if (!no_notify)
  {
    teleNotify.send_msg(str1);
  }
#endif
}

void detect_callback()
{
  Serial.println("DETECT");
  notifyDetection(sensor0.sensNick, sensor0.detCounts);
  turnleds_on();
}

void end_detection_callback()
{
  Serial.println("END_DETECT");
  turnleds_off();
}

void restart_mag()
{
  for (int a = 0; a < logSize; a++)
  {
    sprintf(detectionLog[a], "%s", initword);
  }
}

void update_mag(char *txt)
{
  for (int a = logSize - 2; a >= 0; a--)
  {
    sprintf(detectionLog[a + 1], "%s", detectionLog[a]);
  }
  sprintf(detectionLog[0], "%s", txt);
}

void turnleds_on()
{
  digitalWrite(D3, HIGH);
  on_clock = millis();
}
void turnleds_off()
{
  digitalWrite(D3, LOW);
}

void setup()
{
  startGPIOs();
  turnleds_on();
  startIOTservices();
  startSensors();
  restart_mag();

#if USE_NOTIFY_TELE
  teleNotify.begin(telecmds);
  teleNotify.send_msg("reBoot");
#endif
}

void loop()
{
  iot.looper();
  sensor0.looper();
  if (millis() >= on_clock + ON_TIME * 1000 * 60UL)
  {
    turnleds_off();
  }

#if USE_NOTIFY_TELE
  teleNotify.looper();
#endif

  delay(100);
}
