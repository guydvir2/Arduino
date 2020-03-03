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
#define VER "WEMOS_0.1"
#define USE_NOTIFY_TELE false

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
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

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
#if USE_NOTIFY_TELE
char *Telegram_Nick = DEVICE_TOPIC; //"iotTest";
int time_check_messages = 1;        //sec
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
// ~~~~~~~~~~~ Telegram Notify ~~~~~~~

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
  String command_set[] = {"whois_online", "status", "whoami", "help"};
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
  // else if (in_msg == comp_command[3])
  // {
  //   all_off("Telegram");
  //   sprintf(snd_msg, "%sAll-Off signal was sent", prefix2);
  // } // off
  // else if (in_msg == comp_command[4])
  // {
  // }
  // else if (in_msg == comp_command[5])
  // {
  //   char m1[20];
  //   char m2[20];
  //   sprintf(snd_msg, "%s", prefix2);

  //   for (int i = 0; i < NUM_SWITCHES; i++)
  //   {
  //     if (TO[i]->remain() > 0)
  //     {
  //       TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), m1, m2);
  //       sprintf(t1, "Switch [#%d] already in TimeOut - timeLeft [%s]\n", i, m1);
  //     }
  //     else
  //     {
  //       TO[i]->restart_to();
  //       sprintf(t1, "Switch [#%d] restarting TimeOut\n", i);
  //     }

  //     strcat(snd_msg, t1);
  //   }
  // } // timeout
  else if (in_msg == comp_command[2])
  {
    sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, DEVICE_TOPIC);
  } // whoami
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

    // } // all_commands
    Serial.print("in_msg: ");
    Serial.println(in_msg);
    Serial.print("snd_msg: ");
    Serial.println(snd_msg);
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

// ~~~~~~~~~~~~~~~~~ PIR Sensor ~~~~~~~~~~~~~~
#define PIN_TO_SENSOR_1 D8
#define PIN_TO_SENSOR_2 D7
#define BUZ_PIN 5
#define DET_DURATION 5 // sec in detection

int detCounter = 0;
char lines[2][20];

PIRsensor sensor0(PIN_TO_SENSOR_1, "Sensor_1", 10);
PIRsensor sensor1(PIN_TO_SENSOR_2, "Sensor_2", 10);

void startSensors()
{
  sensor0.use_timer = false;
  sensor0.timer_duration = 15;
  sensor0.ignore_det_interval = 5;
  // sensor0.run_func(quick_buzz);
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

  if (s0 && s1 && detection == false)
  {
    char det_word[100];
    char timeStamp[16];
    char dateStamp[16];

    iot.return_clock(timeStamp);
    iot.return_date(dateStamp);

    detCounter++;
    detection = true;
    sprintf(det_word, "[%s %s] [%s] Detection [#%d]", dateStamp, timeStamp, DEVICE_TOPIC, detCounter);
    iot.pub_msg(det_word);
    #if USE_NOTIFY_TELE
    teleNotify.send_msg(det_word);
    #endif
  }
  else if (s0 == false && s1 == false && detection == true)
  {
    detection = false;
  }
}

void make_buz(byte i, byte del = 50)
{
  for (int x = 0; x < (int)i; x++)
  {
    digitalWrite(BUZ_PIN, HIGH);
    delay(del);
    digitalWrite(BUZ_PIN, LOW);
    delay(del);
    x++;
  }
}
void quick_buzz()
{
  make_buz(1, 5);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void startGPIOs()
{
  pinMode(PIN_TO_SENSOR_1, INPUT);
  pinMode(PIN_TO_SENSOR_2, INPUT);
  pinMode(BUZ_PIN, OUTPUT);
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
