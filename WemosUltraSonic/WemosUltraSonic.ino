#include <Arduino.h>
#include <myIOT.h>
#include <myPIR.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~              // belong to myIOT
#define DEVICE_TOPIC "ultrasonic"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "extLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Hardware ~~~~~~~~~~~~
#define relayPin D3
#define IS_ON HIGH

#define trigPin D2
#define echoPin D1
#define re_trigger_delay 20 // seconds to next detect
#define sensitivity 20      // cm of change 1..350

bool relayState = false;

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"
#define USE_NOTIFY_TELE true
#define TO_NICK "UltraSonic"

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

// ~~~~~~~ ultra-sonic sensor ~~~~~~~~~~~
UltraSonicSensor usensor(trigPin, echoPin, re_trigger_delay, sensitivity);

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
    // start_timer();
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "off") == 0)
  {
    sprintf(msg_MQTT, "Command: Turn [Off]");
    // end_timer();
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
      // restore_timer();
    }
  }
}
void detection()
{
  teleNotify.send_msg("US_DETECT");
}

void setup()
{
  startGPIOs();
  startIOTservices();
  
  usensor.startGPIO();
  usensor.detect_cb(detection);

#if USE_NOTIFY_TELE
      teleNotify.begin(telecmds);
#endif
}

void loop()
{
  iot.looper();
  usensor.check_detect();

#if USE_NOTIFY_TELE
  teleNotify.looper();
#endif

  delay(100);
}
