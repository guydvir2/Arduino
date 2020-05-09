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
#define TELEGRAM_OUT_TOPIC "myHome/Telegram_out"
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
void startPIR(){
  NewPIRsensor.useButton = false;
  NewPIRsensor.usePWM = false;
  NewPIRsensor.RelayON_def = true;
  NewPIRsensor.ButtonPressed_def = LOW;
  NewPIRsensor.SensorDetection_def = LOW;
  // NewPIRsensor.detection_callback()
  NewPIRsensor.start();
}
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

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
        if (NewPIRsensor.swState >= 1.0)
        {
          sendTelegramServer("TEST_PIR_DETECT");
        }
    #endif
  }
}

void setup()
{
  startPIR();
  startIOTservices();
  sendTelegramServer("Boot");
}

void loop()
{
  iot.looper();
  NewPIRsensor.looper();
  notifyMQTT();

  delay(100);
}
