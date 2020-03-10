#include <myIOT.h>
#include <Arduino.h>
#include <myPIR.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~              // belong to myIOT
#define DEVICE_TOPIC "PIR"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "sensors"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ********** Sketch Services  ***********
#define VER "WEMOS_0.2"
#define USE_NOTIFY_TELE true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

bool detection = false;

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
}
void giveStatus(char *outputmsg)
{
  sprintf(outputmsg, "Status: Total detections [#%d]", detection);
}

// ~~~~~~~~~~~~~~~~~ PIR Sensor ~~~~~~~~~~~~~~
#define PIN_TO_SENSOR_1 D5
#define PIN_TO_SENSOR_2 D7
#define DET_DURATION 5 // sec in detection

int detCounter = 0;
char lines[2][20];
bool no_notify = false;

PIRsensor sensor0(PIN_TO_SENSOR_1, "Sensor_1", 10);
PIRsensor sensor1(PIN_TO_SENSOR_2, "Sensor_2", 10);

void startSensors()
{
  sensor0.use_serial = true;
  sensor0.use_timer = false;
  sensor0.timer_duration = DET_DURATION;
  sensor0.ignore_det_interval = 5;
  // sensor0.run_func(notifyDetection);
  sensor0.start();

  sensor1.use_timer = false;
  sensor1.timer_duration = DET_DURATION;
  sensor1.ignore_det_interval = 5;
  // sensor1.run_func(quick_buzz);
  sensor1.start();
}

void sensorLoop()
{
  sensor0.checkSensor(); // triggers logic "1" upon detection.once.
  sensor1.checkSensor();

  bool s0 = sensor0.logic_state; // stays in logic "1" for pre-defined duration
  bool s1 = sensor1.logic_state;

  if (s0 && s1 && !detection)
  {
    detCounter++;
    detection = true;
    notifyDetection(DEVICE_TOPIC, detCounter);
  }
  else if (s0 == false && s1 == false && detection == true)
  {
    detection = false;
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
#if USE_NOTIFY_TELE
char *Telegram_Nick = DEVICE_TOPIC; //"iotTest";
int time_check_messages = 1;        //sec
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
// ~~~~~~~~~~~ Telegram Notify ~~~~~~~

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
  String command_set[] = {"whois_online", "status", "reset", "whoami", "help", 
  "stop_sensor", "restore_sensor", "stop_notify", "restore_notify"};
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
    sprintf(snd_msg, "%s~ stopped", prefix2);
    sensor0.stop_sensor = true;
    sensor1.stop_sensor = true;
  }
  else if (in_msg == comp_command[6])
  {
    sprintf(snd_msg, "%s~ restart", prefix2);
    sensor0.stop_sensor = false;
    sensor1.stop_sensor = false;
  }
  else if (in_msg == comp_command[7])
  {
    sprintf(snd_msg, "%s~ stop notifications", prefix2);
    no_notify = true;
  }
  else if (in_msg == comp_command[8])
  {
    sprintf(snd_msg, "%s~ restart notifications", prefix2);
    no_notify = false;
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
  pinMode(PIN_TO_SENSOR_2, INPUT);
}

void notifyDetection(char *device, int counter)
{
  char det_word[100];
  char timeStamp[16];
  char dateStamp[16];

  iot.return_clock(timeStamp);
  iot.return_date(dateStamp);

  sprintf(det_word, "[%s %s] [%s] Detection [#%d]", dateStamp, timeStamp, device, counter);
  iot.pub_msg(det_word);
  Serial.println(det_word);

#if USE_NOTIFY_TELE
  if (!no_notify)
  {
    teleNotify.send_msg(det_word);
  }
#endif
}

void setup()
{
  startGPIOs();
  startIOTservices();
  startSensors();

#if USE_NOTIFY_TELE
  teleNotify.begin(telecmds);
#endif
}

void loop()
{
  iot.looper();
  sensorLoop();

#if USE_NOTIFY_TELE
  teleNotify.looper();
#endif

  delay(500);
}
