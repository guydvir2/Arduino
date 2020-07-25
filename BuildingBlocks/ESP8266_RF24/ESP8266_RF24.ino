#include <myIOT.h>
#include <Arduino.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~                        // belong to myIOT
#define DEVICE_TOPIC "RF24"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "TESTS"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false     // Serial Monitor
#define USE_WDT true         // watchDog resets
#define USE_OTA true         // OTA updates
#define USE_RESETKEEPER true // detect quick reboot and real reboots
#define USE_FAILNTP true     // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT ~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

#define USE_RADIO true
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

  // if (strcmp(income_msg, "status") == 0)
  // {
  //   giveStatus(msg_MQTT);
  //   iot.pub_msg(msg_MQTT);
  // }
  // else if (strcmp(income_msg, "ver") == 0)
  // {
  //   sprintf(msg_MQTT, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
  //   iot.pub_msg(msg_MQTT);
  //   sprintf(msg_MQTT, "ver #2: DailyTO[%d], UseInputs[%d], ON_AT_BOOT[%d], Use_Sensor[%d], Use_Telegram[%d], HardReboot[%d]",
  //           USE_DAILY_TO, USE_INPUTS, ON_AT_BOOT, USE_SENSOR, USE_NOTIFY_TELE, HARD_REBOOT);
  //   iot.pub_msg(msg_MQTT);
  // }
  // else if (strcmp(income_msg, "help") == 0)
  // {
  //   sprintf(msg_MQTT, "Help: Commands #1 - [on, off, flash, format]");
  //   iot.pub_msg(msg_MQTT);
  //   sprintf(msg_MQTT, "Help: Commands #2 - [remain, restartTO, timeout(x), endTO, updateTO(x), restoreTO, statusTO]");
  //   iot.pub_msg(msg_MQTT);
  //   sprintf(msg_MQTT, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
  //   iot.pub_msg(msg_MQTT);
  //   sprintf(msg_MQTT, "Help: Commands #4 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
  //   iot.pub_msg(msg_MQTT);
  // }
  // else if (strcmp(income_msg, "flash") == 0)
  // {
  //   TO[0]->inCodeTimeOUT_inFlash.printFile();
  // }
  // else if (strcmp(income_msg, "format") == 0)
  // {
  //   TO[0]->inCodeTimeOUT_inFlash.format();
  // }
  // else if (strcmp(income_msg, "all_off") == 0)
  // {
  //   all_off("MQTT");
  // }
  // else
  // {
  //   int num_parameters = iot.inline_read(income_msg);

  //   if (strcmp(iot.inline_param[1], "on") == 0)
  //   {
  //     switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false);
  //   }
  //   else if (strcmp(iot.inline_param[1], "off") == 0)
  //   {
  //     switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false);
  //   }
  //   else if (strcmp(iot.inline_param[1], "timeout") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
  //     TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg_MQTT);
  //     sprintf(msg_MQTT, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "updateTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
  //     sprintf(msg_MQTT, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
  //     iot.pub_msg(msg_MQTT);
  //     delay(1000);
  //     iot.notifyOffline();
  //     iot.sendReset("TimeOut update");
  //   }
  //   else if (strcmp(iot.inline_param[1], "remain") == 0)
  //   {
  //     if (TO[atoi(iot.inline_param[0])]->remain() > 0)
  //     {
  //       TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TO[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg_MQTT);
  //       sprintf(msg_MQTT, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
  //       iot.pub_msg(msg_MQTT);
  //     }
  //   }
  //   else if (strcmp(iot.inline_param[1], "restartTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->restart_to();
  //     sprintf(msg_MQTT, "TimeOut: Switch [#%d] [Restart]", atoi(iot.inline_param[0]));
  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "statusTO") == 0)
  //   {
  //     sprintf(msg_MQTT, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
  //             "TimeOut", atoi(iot.inline_param[0]),
  //             TIMEOUTS[atoi(iot.inline_param[0])],
  //             TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
  //             TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode");
  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "endTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->endNow();
  //     sprintf(msg_MQTT, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "restoreTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->restore_to();
  //     TO[atoi(iot.inline_param[0])]->restart_to();
  //     sprintf(msg_MQTT, "TimeOut: Switch [#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
  //     iot.pub_msg(msg_MQTT);
  //     iot.notifyOffline();
  //     iot.sendReset("Restore");
  //   }
  //   else if (strcmp(iot.inline_param[1], "on_dailyTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->dailyTO.on[0] = atoi(iot.inline_param[2]); //hours
  //     TO[atoi(iot.inline_param[0])]->dailyTO.on[1] = atoi(iot.inline_param[3]); // minutes
  //     TO[atoi(iot.inline_param[0])]->dailyTO.on[2] = atoi(iot.inline_param[4]); // seconds

  //     TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

  //     sprintf(msg_MQTT, "%s: Switch [#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
  //             TO[atoi(iot.inline_param[0])]->dailyTO.on[0], TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.on[2]);

  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "off_dailyTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
  //     TO[atoi(iot.inline_param[0])]->dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
  //     TO[atoi(iot.inline_param[0])]->dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

  //     TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

  //     sprintf(msg_MQTT, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
  //             TO[atoi(iot.inline_param[0])]->dailyTO.off[0], TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.off[2]);

  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "flag_dailyTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->dailyTO.flag = atoi(iot.inline_param[2]);
  //     TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

  //     sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
  //             atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "useflash_dailyTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->dailyTO.useFlash = atoi(iot.inline_param[2]);
  //     TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

  //     sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
  //             atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "status_dailyTO") == 0)
  //   {
  //     sprintf(msg_MQTT, "%s: Switch [#%d] {ON:%02d:%02d:%02d} {OFF:%02d:%02d:%02d} {Flag:%s} {Values:%s}",
  //             clockAlias, atoi(iot.inline_param[0]),
  //             TO[atoi(iot.inline_param[0])]->dailyTO.on[0],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.on[2],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.off[0],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.off[2],
  //             TO[atoi(iot.inline_param[0])]->dailyTO.flag ? "ON" : "OFF",
  //             TO[atoi(iot.inline_param[0])]->dailyTO.useFlash ? "Flash" : "inCode");
  //     iot.pub_msg(msg_MQTT);
  //   }
  //   else if (strcmp(iot.inline_param[1], "restart_dailyTO") == 0)
  //   {
  //     TO[atoi(iot.inline_param[0])]->restart_dailyTO(TO[atoi(iot.inline_param[0])]->dailyTO);
  //     sprintf(msg_MQTT, "%s: Switch[#%d] Resume daily Timeout", clockAlias, atoi(iot.inline_param[0]));
  //     iot.pub_msg(msg_MQTT);
  //   }

  //   else
  //   {
  //     if (strcmp(income_msg, "offline") != 0 && strcmp(income_msg, "online") != 0 && strcmp(income_msg, "resetKeeper") != 0)
  //     {
  //       sprintf(msg_MQTT, "Unrecognized Command: [%s]", income_msg);
  //       iot.pub_log(msg_MQTT);
  //     }
  //   }
  //   for (int n = 0; n <= num_parameters - 1; n++)
  //   {
  //     sprintf(iot.inline_param[n], "");
  //   }
  // }
}
void giveStatus(char *outputmsg)
{
  char t1[50];
  char t2[50];
  char t3[50];

  // sprintf(t3, "Status: ");
  // for (int i = 0; i < NUM_SWITCHES; i++)
  // {
  //   if (TO[i]->remain() > 0)
  //   {
  //     TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), t2, t1);
  //     sprintf(t1, "timeLeft[%s]", t2);
  //   }
  //   else
  //   {
  //     sprintf(t1, "");
  //   }
  //   sprintf(t2, "Switch[#%d] [%s] %s ", i, digitalRead(relays[i]) ? "ON" : "OFF", t1);
  //   strcat(t3, t2);
  // }
  // sprintf(outputmsg, "%s", t3);
}

// ~~~~~~~~~~~~~~ RF24 ~~~~~~~~~~~~~~
#if USE_RADIO
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

int CE_Pin = 7;
int CSN_Pin = 8;
const byte address[6] = "00001";
char incoming_radio_txt[32];

RF24 radio(CE_Pin, CSN_Pin);

void startRadio_listening()
{
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MIN; RF24_PA_MAX
  radio.setChannel(35);
  radio.setAutoAck(0);

  radio.openReadingPipe(0, address);
  radio.startListening();
}

void startRadio_sending()
{
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MIN; RF24_PA_LOW
  radio.setRetries(2, 15);
  radio.setChannel(35);
  radio.setAutoAck(0);

  radio.openWritingPipe(address);
  radio.stopListening();
}

void Radio_loop()
{
  if (radio.available())
  {
    radio.read(&incoming_radio_txt, sizeof(incoming_radio_txt));
  }
}
#endif

// ~~~~~~~~~~~~ END NRF24 ~~~~~~~~~~~~~~

void setup()
{
  startIOTservices();
  #if USE_RADIO
  
}

void loop()
{
  iot.looper();
}
