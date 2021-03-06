#include <myIOT.h>
#include <RCSwitch.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
#define USE_TELEGRAM true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "Pergola_Rx"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "RF433"
#define TELEGRAM_OUT_TOPIC "Telegram_out"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ************ 433MHz Receiver ****************
#define ID_IDENTIFY 1;
const int buzzerPin = 0;
const int RxPin = 2;
const int msg_retries = 3;
const char *RF_REMOTES[] = {"Shachar", "Anna", "Guy"};
const int rf_base[4] = {3, 7, 11, 13};
const int timeout_between_commands = 5; //seconds
const char *comm_desc[] = {"Alarm - disarmed", "Saloon Exit Up", "Alarm - armed away", "All Windows Down"};
const char *comm_mqtt[] = {"myHome/alarmMonitor", "disarmed", "myHome/Windows/saloonExit", "up", "myHome/alarmMonitor", "armed_away", "myHome/Windows", "down"};

RCSwitch RF_Rx = RCSwitch();

void makeBeep(int t = 20)
{
  digitalWrite(buzzerPin, HIGH);
  delay(t);
  digitalWrite(buzzerPin, LOW);
}
void confirmBeep(int t = 100)
{
  makeBeep();
  delay(t);
  makeBeep();
  delay(t);
  makeBeep(450);
}
void send_commands_cb(char *msg, int i)
{
  iot.mqttClient.publish(comm_mqtt[2 * i], comm_mqtt[2 * i + 1]);
  iot.send_tele_msg(msg);
  confirmBeep();
}
void Rx_looper()
{
  if (RF_Rx.available())
  {
    static unsigned long last_command_clock = 0;
    static unsigned long last_recieved_command = 0;
    static int last_command = 0;
    static int command_counter = 0;

    unsigned long read_rf = RF_Rx.getReceivedValue();

    //    Serial.print("Received ");
    //    Serial.print(RF_Rx.getReceivedValue());
    //    Serial.print(" / ");
    //    Serial.print(RF_Rx.getReceivedBitlength());
    //    Serial.print("bit ");
    //    Serial.print("Protocol: ");
    //    Serial.println(RF_Rx.getReceivedProtocol());
    //    RF_Rx.resetAvailable();
    if (last_recieved_command != 0 && millis() - last_recieved_command >= timeout_between_commands * 1000)// timeout to filter false sporadic commands
    {
      command_counter = 0;
      last_recieved_command = 0;
    }

    if (millis() - last_command_clock >= timeout_between_commands * 1000) // timeout to avoid burst of commands
    {
      for (int i = 0; i < 4; i++)
      {
        if (read_rf % rf_base[i] == 0)
        {
          int x = read_rf / rf_base[i]; //which remote
          last_recieved_command = millis();
          if (rf_base[i] == last_command)
          {
            command_counter++;
            makeBeep();
          }
          else
          {
            command_counter = 1;
            last_command = rf_base[i];
          }
          if (command_counter >= msg_retries)
          {
            char t[50];
            sprintf(t, "[%s]'s Remote pressed [%s]", RF_REMOTES[x - 1], comm_desc[i]);
            send_commands_cb(t, i);
            last_command_clock = millis();
            command_counter = 0;
          }
        }
      }
      RF_Rx.resetAvailable();
    }
  }
  else
  {
    RF_Rx.resetAvailable();
  }
}

// ***************** End 433MHz ****************
void startIOTservices()
{
  iot.useSerial = USE_SERIAL;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.useResetKeeper = USE_RESETKEEPER;
  iot.resetFailNTP = USE_FAILNTP;
  iot.useTelegram = USE_TELEGRAM;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  strcpy(iot.telegramServer, TELEGRAM_OUT_TOPIC);
  iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: Im OK, thank you for asking");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d], Telegram[%d]",
            VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP, USE_TELEGRAM);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help") == 0)
  {
    sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg);
  }
}

void setup()
{
  startIOTservices();
  RF_Rx.enableReceive(RxPin);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}
void loop()
{
  iot.looper();
  Rx_looper();
  delay(100);
}
