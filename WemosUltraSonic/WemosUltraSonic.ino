#include <Arduino.h>
#include <myIOT.h>
#include <myPIR.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~              // belong to myIOT
#define DEVICE_TOPIC "frontDoorLEDs"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "extLights"
#define TELEGRAM_OUT_TOPIC "myHome/Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ********** Sketch Services  ***********
#define VER "WEMOS_2.0"
#define USE_NOTIFY_TELE false
#define MAX_ON_TIME 60 * 6 // minutes -- this timeout overides all

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

// ~~~~~~~ ultra-sonic sensor ~~~~~~~~~~~
#define trigPin D7
#define echoPin D1
#define re_trigger_delay 60 // seconds to next detect
#define sensitivity 5       // dist change between 2 readings, considered as detection. cm of change 1..350

UltraSonicSensor usensor(trigPin, echoPin, re_trigger_delay, sensitivity);

void start_usSensor()
{
  usensor.startGPIO();
  usensor.detect_cb(detection);
  usensor.min_dist_trig = 20;
  usensor.max_dist_trig = 200;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES 1
#define TIMEOUT_SW0 2 * 60 // mins for SW0
#define ON_AT_BOOT false   // On or OFF at boot (Usually when using inputs, at boot/PowerOn - state should be off
#define USE_DAILY_TO true

const int START_dailyTO[] = {20, 0, 0};
const int END_dailyTO[] = {23, 55, 0};
int TIMEOUTS[NUM_SWITCHES] = {TIMEOUT_SW0};

timeOUT timeOut_SW0("SW0", TIMEOUTS[0]);
timeOUT *TO[] = {&timeOut_SW0};
char *clockAlias = "Daily TimeOut";

void startTO()
{
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    TO[i]->begin(ON_AT_BOOT);
    start_dailyTO(i);
  }
}
void start_dailyTO(byte i)
{
  memcpy(TO[i]->dailyTO.on, START_dailyTO, sizeof(START_dailyTO));
  memcpy(TO[i]->dailyTO.off, END_dailyTO, sizeof(END_dailyTO));
  TO[i]->dailyTO.flag = USE_DAILY_TO;
  TO[i]->check_dailyTO_inFlash(TO[i]->dailyTO, i);
}
void notify_dailyTO(byte i)
{
  // publish a ready made notification
  if (strcmp(TO[i]->dTO_pubMsg, "") != 0)
  {
    iot.pub_msg(TO[i]->dTO_pubMsg);
    sprintf(TO[i]->dTO_pubMsg, "");
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Switch/ Relay Hardware
#define relayPin D3
#define RelayOn HIGH
#define USE_PWM false

int relays[] = {relayPin};
bool boot_overide[] = {true, true};

void switchIt(char *txt1, int sw_num, bool state, char *txt2 = "", bool show_timeout = true, int PWMval = -1)
{
  char msg[50], msg1[50], msg2[50], states[50], tempstr[50];
  char *word = {"Turned"};

  if (digitalRead(relays[sw_num]) != state || boot_overide[sw_num] == true || (USE_PWM && PWMval >= 0))
  {
    if (USE_PWM)
    {
      // if (pwmState) // already in ON state
      // {
      //   if (PWMval == 0 || PWMval > maxPWM)
      //   {
      //     sprintf(msg, "%s: Switch[#%d] %s[%s]", txt1, sw_num, word, state ? "ON" : "OFF");
      //     TO[sw_num]->endNow();
      //     pwmState = false;
      //   }
      //   else if (PWMval > 0 && PWMval <= maxPWM)
      //   {
      //     sprintf(msg, "%s: Switch[#%d] power[%d%%] -> [%d%%]", txt1, sw_num, PWMvalue, PWMval);

      //     char atemp[10];
      //     sprintf(atemp, "%d%% Power", PWMval);
      //   }
      // }
      // else // in OFF state
      // {
      //   pwmState = true;
      //   sprintf(msg, "%s: Switch[#%d] %s[%s] power[%d%%]", txt1, sw_num, word, state ? "ON" : "OFF", PWMval);

      //   char atemp[10];
      //   sprintf(atemp, "%d%% Power", PWMval);
      // }
      // analogWrite(relays[sw_num], PWMval * PWM_RES / 100);
      // PWMvalue = PWMval;
    }
    else
    {
      digitalWrite(relays[sw_num], state);
    }

    TO[sw_num]->convert_epoch2clock(now() + TO[sw_num]->remain(), now(), msg1, msg2);
    if (boot_overide[sw_num] == true)
    {
      if (iot.mqtt_detect_reset == 1 || TO[sw_num]->remain() > 0)
      { //BOOT TIME ONLY for after quick boot
        word = {"Resume"};
        boot_overide[sw_num] = false;
      }
    }
    if (PWMval < 0)
    {
      sprintf(msg, "%s: Switch[#%d] %s[%s] %s", txt1, sw_num, word, state ? "ON" : "OFF", txt2);
    }

    if (state == 1 && show_timeout)
    {
      sprintf(msg2, "timeLeft[%s]", msg1);
      strcat(msg, msg2);
    }

    iot.pub_msg(msg);

    sprintf(states, "");
    for (int i = 0; i < NUM_SWITCHES; i++)
    {
      sprintf(states, "[%s]", digitalRead(relays[i]) ? "ON" : "OFF");
      iot.pub_state(states, i);
    }
  }
}
void checkRebootState(byte i = 0)
{
  static bool checkreboot = true;
  int rebstate = iot.mqtt_detect_reset;

  if (rebstate != 2 && checkreboot)
  {
    checkreboot = false;
    if (rebstate == 0 && ON_AT_BOOT)
    {
      // PowerOn - not a quickReboot
      TO[i]->restart_to();
      iot.pub_log("--> NormalBoot & On-at-Boot. Restarting TimeOUT");
    }
    else if (TO[i]->looper() == 0)
    { // was not during TO
      if (rebstate == 1)
      {
        iot.pub_log("--> PowerLoss Boot");
      }
      digitalWrite(relays[i], !RelayOn);
      iot.pub_log("--> Stopping Quick-PowerON");
      for (int a = 0; a < NUM_SWITCHES; a++)
      {
        boot_overide[a] = false;
      }
    }
    else
    {
      iot.pub_log("--> Continue unfinished TimeOuts");
      boot_overide[i] = true;
    }
  }
}
void TO_looper(byte i = 0)
{
  static bool last_relState[] = {0, 0};
  static bool relState[] = {0, 0};

  if (iot.mqtt_detect_reset != 2)
  {
    relState[i] = TO[i]->looper();
    if (relState[i] != last_relState[i])
    {
      // change state (ON <-->OFF)
      notify_dailyTO(i);
      switchIt("TimeOut", i, relState[i]);
    }
  }
  last_relState[i] = relState[i];

  checkRebootState();
}

void flicker(byte i = 20)
{
  bool lightstatus = digitalRead(relays[0]);
  for (int x = 0; x < i; x++)
  {
    lightstatus = !lightstatus;
    digitalWrite(relays[0], lightstatus);
    delay(100);
  }
}
void max_on_breaker(int max_timeout)
{
  static bool last_state;
  static unsigned long timeout = !RelayOn;
  bool current_state = digitalRead(relays[0]);

  if (timeout != 0 && (millis() - timeout) > max_timeout * 1000 * 60)
  {
    switchIt("Breaker", 0, 0, "Max On time reached");
  }

  if (last_state != current_state)
  {
    if (current_state == RelayOn)
    {
      timeout = millis();
      // Serial.println("ON DETECED");
    }
    else
    {
      timeout = 0;
      // Serial.println("OFF DETECTED");
    }
    last_state = current_state;
  }
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
    sprintf(msg_MQTT, "ver #2: DailyTO[%d], ON_AT_BOOT[%d], Use_Telegram[%d]", USE_DAILY_TO, ON_AT_BOOT, USE_NOTIFY_TELE);
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "help") == 0)
  {
    sprintf(msg_MQTT, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg_MQTT);
    sprintf(msg_MQTT, "Help: Commands #2 - [i,remain; i,restartTO; i,timeout,x; i,endTO; i,updateTO,x; i,restoreTO,x; i,statusTO]");
    iot.pub_msg(msg_MQTT);
    sprintf(msg_MQTT, "Help: Commands #3 - [on, off, flash, format]");
    iot.pub_msg(msg_MQTT);
    sprintf(msg_MQTT, "Help: Commands #4 - [i,off_dailyTO,h,m,s; i,on_dailyTO,h,m,s; i,flag_dailyTO,true/false]");
    iot.pub_msg(msg_MQTT);
    sprintf(msg_MQTT, "Help: Commands #5 - [i,useflash_dailyTO,true/false; i,status_dailyTO]");
    iot.pub_msg(msg_MQTT);
  }
  else if (strcmp(income_msg, "flash") == 0)
  {
    TO[0]->inCodeTimeOUT_inFlash.printFile();
  }
  else if (strcmp(income_msg, "format") == 0)
  {
    TO[0]->inCodeTimeOUT_inFlash.format();
  }
  else if (strcmp(income_msg, "all_off") == 0)
  {
    all_off("MQTT");
  }
  else
  {
    int num_parameters = iot.inline_read(income_msg);

    if (strcmp(iot.inline_param[1], "on") == 0)
    {
      if (USE_PWM)
      {
        // switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false, defaultPWM);
      }
      else
      {
        switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false);
      }
    }
    else if (strcmp(iot.inline_param[1], "off") == 0)
    {
      if (USE_PWM)
      {
        // switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false, 0.0);
      }
      else
      {
        switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false);
      }
    }
    else if (strcmp(iot.inline_param[1], "timeout") == 0)
    {
      TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
      TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg_MQTT);
      sprintf(msg_MQTT, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "updateTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
      sprintf(msg_MQTT, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
      iot.pub_msg(msg_MQTT);
      delay(1000);
      iot.notifyOffline();
      iot.sendReset("TimeOut update");
    }
    else if (strcmp(iot.inline_param[1], "remain") == 0)
    {
      if (TO[atoi(iot.inline_param[0])]->remain() > 0)
      {
        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TO[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg_MQTT);
        sprintf(msg_MQTT, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
        iot.pub_msg(msg_MQTT);
      }
    }
    else if (strcmp(iot.inline_param[1], "restartTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->restart_to();
      sprintf(msg_MQTT, "TimeOut: Switch [#%d] [Restart]", atoi(iot.inline_param[0]));
      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "statusTO") == 0)
    {
      sprintf(msg_MQTT, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
              "TimeOut", atoi(iot.inline_param[0]),
              TIMEOUTS[atoi(iot.inline_param[0])],
              TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
              TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode");
      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "endTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->endNow();
      sprintf(msg_MQTT, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "restoreTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->restore_to();
      TO[atoi(iot.inline_param[0])]->restart_to();
      sprintf(msg_MQTT, "TimeOut: Switch [#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
      iot.pub_msg(msg_MQTT);
      iot.notifyOffline();
      iot.sendReset("Restore");
    }
    else if (strcmp(iot.inline_param[1], "on_dailyTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->dailyTO.on[0] = atoi(iot.inline_param[2]); //hours
      TO[atoi(iot.inline_param[0])]->dailyTO.on[1] = atoi(iot.inline_param[3]); // minutes
      TO[atoi(iot.inline_param[0])]->dailyTO.on[2] = atoi(iot.inline_param[4]); // seconds

      TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

      sprintf(msg_MQTT, "%s: Switch [#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
              TO[atoi(iot.inline_param[0])]->dailyTO.on[0], TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
              TO[atoi(iot.inline_param[0])]->dailyTO.on[2]);

      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "off_dailyTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
      TO[atoi(iot.inline_param[0])]->dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
      TO[atoi(iot.inline_param[0])]->dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

      TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

      sprintf(msg_MQTT, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
              TO[atoi(iot.inline_param[0])]->dailyTO.off[0], TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
              TO[atoi(iot.inline_param[0])]->dailyTO.off[2]);

      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "flag_dailyTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->dailyTO.flag = atoi(iot.inline_param[2]);
      TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

      sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
              atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "useflash_dailyTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->dailyTO.useFlash = atoi(iot.inline_param[2]);
      TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

      sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
              atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "status_dailyTO") == 0)
    {
      sprintf(msg_MQTT, "%s: Switch [#%d] {ON:%02d:%02d:%02d} {OFF:%02d:%02d:%02d} {Flag:%s} {Values:%s}",
              clockAlias, atoi(iot.inline_param[0]),
              TO[atoi(iot.inline_param[0])]->dailyTO.on[0],
              TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
              TO[atoi(iot.inline_param[0])]->dailyTO.on[2],
              TO[atoi(iot.inline_param[0])]->dailyTO.off[0],
              TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
              TO[atoi(iot.inline_param[0])]->dailyTO.off[2],
              TO[atoi(iot.inline_param[0])]->dailyTO.flag ? "ON" : "OFF",
              TO[atoi(iot.inline_param[0])]->dailyTO.useFlash ? "Flash" : "inCode");
      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "restart_dailyTO") == 0)
    {
      TO[atoi(iot.inline_param[0])]->restart_dailyTO(TO[atoi(iot.inline_param[0])]->dailyTO);
      sprintf(msg_MQTT, "%s: Switch[#%d] Resume daily Timeout", clockAlias, atoi(iot.inline_param[0]));
      iot.pub_msg(msg_MQTT);
    }
    else if (strcmp(iot.inline_param[1], "change_pwm") == 0)
    {
      switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false, atoi(iot.inline_param[2]));
    }

    else
    {
      if (strcmp(income_msg, "offline") != 0 && strcmp(income_msg, "online") != 0 && strcmp(income_msg, "resetKeeper") != 0)
      {
        sprintf(msg_MQTT, "Unrecognized Command: [%s]", income_msg);
        iot.pub_log(msg_MQTT);
      }
    }
    for (int n = 0; n <= num_parameters - 1; n++)
    {
      sprintf(iot.inline_param[n], "");
    }
  }
}
void giveStatus(char *outputmsg)
{
  char t1[50];
  char t2[50];
  char t3[50];

  sprintf(t3, "Status: ");
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    if (TO[i]->remain() > 0)
    {
      TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), t2, t1);
      sprintf(t1, "timeLeft[%s]", t2);
    }
    else
    {
      sprintf(t1, "");
    }
    if (USE_PWM)
    {
      // sprintf(t2, "Switch[#%d] power[%d%%] %s", i, PWMvalue, t1);
    }
    else
    {
      sprintf(t2, "Switch[#%d] [%s] %s ", i, digitalRead(relays[i]) ? "ON" : "OFF", t1);
    }
    strcat(t3, t2);
  }
  sprintf(outputmsg, "%s", t3);
}
void all_off(char *from)
{
  char t[50];
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    if (TO[i]->remain() > 0 && relays[i] == RelayOn)
    {
      TO[i]->endNow();
    }
    else if (TO[i]->remain() == 0 && relays[i] == RelayOn)
    {
      switchIt(from, i, false, "", false);
    }
    else if (TO[i]->remain() > 0 && relays[i] != RelayOn)
    {
      TO[i]->endNow();
    }
  }
  sprintf(t, "All OFF: Received from %s", from);
  iot.pub_msg(t);
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
void sendTelegramServer(char *msg, char *tele_server = TELEGRAM_OUT_TOPIC)
{
  char t[200];
  iot.get_timeStamp(now());
  sprintf(t, "[%s][%s]: %s", iot.timeStamp, iot.deviceTopic, msg);
  iot.mqttClient.publish(tele_server, t);
}
void detection()
{
  int time_on_detection = 2; // minutes
  flicker();
  iot.pub_msg("Detection");
  sendTelegramServer("Detection");
   if (TO[0]->remain() == 0)
  {
    TO[0]->setNewTimeout(time_on_detection);
  }
}

void startGPIOs()
{
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    pinMode(relays[i], OUTPUT);
  }
}

void setup()
{
  startGPIOs();
  start_usSensor();
  startIOTservices();
  sendTelegramServer("Boot");
  startTO();
}

void loop()
{
  iot.looper();
  TO_looper();
  usensor.check_detect();

  max_on_breaker(MAX_ON_TIME);
  delay(100);
}
