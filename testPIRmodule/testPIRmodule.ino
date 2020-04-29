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
#define DEVICE_TOPIC "testPIR"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "intLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ±±±±±±±±±±±± Sensor PIR ±±±±±±±±±±±±±±±±±
#define Pin_Sensor_0 D1
#define Pin_Switch_0 D2
#define Pin_extbut_0 D3 // fake - No EXT Button here
#define SwitchTimeOUT_0 1

SensorSwitch NewPIRsensor(Pin_Sensor_0, Pin_Switch_0, SwitchTimeOUT_0, Pin_extbut_0);
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

// ~~~~~~~~~~~  Telegram Notification ~~~~~~~
#if USE_NOTIFY_TELE

char *Telegram_Nick = DEVICE_TOPIC; //"iotTest";
int time_check_messages = 1;        //sec

myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
  String command_set[] = {"whois_online", "status", "reset", "help", "whoami"};
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
    // giveStatus(t1);
    sprintf(snd_msg, "%s%s", prefix2, t1);
  } // status
  else if (in_msg == comp_command[2])
  {
    sprintf(snd_msg, "%s", prefix2);
    // iot.sendReset("Telegram");
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
    sprintf(snd_msg, "%s~%s~ is %s", prefix2, Telegram_Nick, DEVICE_TOPIC);
  } // whoami
}
#endif
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
    sprintf(msg, "Status: Device is [%s]", NewPIRsensor.swState ? "On" : "Off");
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
  // else if (strcmp(incoming_msg, "all_off") == 0)
  // {
  //         for (int i = 0; i < NUM_SW; i++)
  //         {
  //                 if (s[i]->swState != 0.0)
  //                 {
  //                         s[i]->turnOff();
  //                         sprintf(msg, "MQTT: LedStrip [%s] Turned [Off]", ledNames[i]);
  //                         iot.pub_msg(msg);
  //                 }
  //         }
  // }
  // else if (strcmp(incoming_msg, "remain") == 0)
  // {
  //         for (int i = 0; i < NUM_SW; i++)
  //         {
  //                 if (s[i]->timeoutRem > 0)
  //                 {
  //                         sprintf(msg, "MQTT: Remain Time LedStrip [%s] ,[%d] sec", ledNames[i], s[i]->timeoutRem);
  //                         iot.pub_msg(msg);
  //                 }
  //                 else
  //                 {
  //                         sprintf(msg, "MQTT: LedStrip [%s] is [Off]", ledNames[i]);
  //                         iot.pub_msg(msg);
  //                 }
  //         }
  // }

  // else
  // {
  //         int num_parameters = iot.inline_read(incoming_msg);
  //         int x = atoi(iot.inline_param[0]);

  //         if (strcmp(iot.inline_param[1], "on") == 0 && x < NUM_SW)
  //         {
  //                 s[x]->turnOn();
  //         }
  //         else if (strcmp(iot.inline_param[1], "off") == 0 && x < NUM_SW)
  //         {
  //                 s[x]->turnOff();
  //         }
  //         if (strcmp(iot.inline_param[1], "startTO") == 0 && x < NUM_SW)
  //         {
  //                 s[x]->turnOn(atoi(iot.inline_param[2]));
  //         }
  // }
}
void notifyMQTT()
{
  static float lastval = 0.0;
  char msg[50];

  if (NewPIRsensor.swState != lastval)
  {
    lastval = NewPIRsensor.swState;
    sprintf(msg, "Change:now [%s]", NewPIRsensor.swState ? "On" : "Off");
    iot.pub_msg(msg);
#if USE_NOTIFY_TELE
    if (NewPIRsensor.swState > 1.0)
    {
      teleNotify.send_msg("FrontDoor Detection");
    }
#endif
  }
}

void setup()
{
  NewPIRsensor.useButton = false;
  NewPIRsensor.usePWM = false;
  NewPIRsensor.RelayON_def = true;
  NewPIRsensor.ButtonPressed_def = LOW;
  NewPIRsensor.SensorDetection_def = LOW;
  NewPIRsensor.start();

  startIOTservices();
#if USE_NOTIFY_TELE
  teleNotify.begin(telecmds);
  teleNotify.send_msg("FrontDoor Start");
#endif

  // Serial.begin(9600);
}

void loop()
{
  iot.looper();
  NewPIRsensor.looper();
  notifyMQTT();

#if USE_NOTIFY_TELE
  teleNotify.looper();
#endif

  delay(100);
}
