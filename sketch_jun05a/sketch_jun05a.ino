#include <myIOTesp32.h>
#include <myESP32sleep.h>

#define VER "ESP32_0.6v"
#define USE_VMEASURE false
#define USE_SLEEP true
// ~~~~~~~ myIOT32 ~~~~~~~~
#define DEVICE_TOPIC "ESP32_light"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "solarPower"
#define MQTT_TELEGRAM "myHome/Telegram"
// #define MQTT_EXT_TOPIC MQTT_PREFIX "/" MQTT_GROUP "/" DEVICE_TOPIC "/" \
//                                    "onWake"
#define MQTT_EXT_TOPIC "myHome/solarPower/ESP32_light/onWake"
#define USE_SERIAL true
#define USE_OTA true
#define USE_WDT false
#define USE_EXT_TOPIC false
#define USE_RESETKEEPER true

myIOT32 iot(DEVICE_TOPIC);

void startIOT_services()
{
  iot.useSerial = USE_SERIAL;
  iot.useResetKeeper = USE_RESETKEEPER;
  iot.useWDT = USE_WDT;
  iot.useOTA = USE_OTA;
  iot.useExtTopic = USE_EXT_TOPIC;
  iot.ext_mqtt_cb = ext_MQTT;
  strcpy(iot.prefixTopic, MQTT_PREFIX);
  strcpy(iot.addGroupTopic, MQTT_GROUP);
  strcpy(iot.telegramServer, MQTT_TELEGRAM);
  strcpy(iot.extTopic, MQTT_EXT_TOPIC);

  iot.start();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Sleep ~~~~~~~~~~~
#define SLEEP_TIME 1
#define FORCE_AWAKE_TIME 15
#define NO_SLEEP_TIME 4
#define DEV_NAME DEVICE_TOPIC
bool no_sleep_flag = false;

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);
void b4sleep()
{
  Serial.println("b4Sleep");
  iot.getTime();
  // postWake();
}
void startSleep_services()
{
  go2sleep.run_func(b4sleep); // define a function to be run prior to sleep.
  go2sleep.no_sleep_minutes = NO_SLEEP_TIME;

  iot.getTime(); // generate clock and passing it to next func.
  go2sleep.startServices();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~ Power Managment ~~~~
const int measureVoltagePin = 35;
const int ADC_res = 4095;
const float Rvalue = 0.45; //0.75
const float vcc = 3.3;
float bat_volt = 0.0;

void bat_measure()
{
  byte sample = 5;
  bat_volt = 0.0;
  for (int i = 0; i < sample; i++)
  {
    bat_volt += analogRead(measureVoltagePin);
  }
  bat_volt = ((bat_volt / ((float)sample)) / ADC_res) * vcc * (1.0 / Rvalue);
}

// bool checkWake_topic()
// {
//   if (strcmp(iot.mqtt_msg.msg, "maintainance") == 0)
//   {
//     sprintf(iot.mqtt_msg.msg, "");
//   }
// }
void postWake()
{
  StaticJsonDocument<300> doc;
  // Constansts
  doc["deviceName"] = go2sleep.WakeStatus.name;
  doc["bootCount"] = go2sleep.WakeStatus.bootCount;
  doc["forcedAwake"] = go2sleep.WakeStatus.awake_duration;

  // Per wake cycle
  doc["sleepDuration"] = go2sleep.sleepduration;
  doc["Wake"] = go2sleep.WakeStatus.wake_clock;
  doc["SleetStart"] = go2sleep.WakeStatus.startsleep_clock;
  doc["nextWake"] = go2sleep.WakeStatus.nextwake_clock;
  doc["RTCdrift"] = go2sleep.WakeStatus.drift_err;
  doc["WakeErr"] = go2sleep.WakeStatus.wake_err;

  char a[250];
  String output;
  serializeJson(doc, output);
  output.toCharArray(a, output.length() + 1);
  Serial.println(a);
  iot.pub_ext(a, true);
}
void start_maintainance()
{
  no_sleep_flag = true;
  iot.pub_ext("maintainance_started", true);
}

void ext_MQTT(char *incoming_msg)
{
  char msg[200];
  char msg2[20];

  if (strcmp(incoming_msg, "status") == 0)
  {
    // giveStatus(msg);
    iot.pub_msg("msg");
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver #1: Code: [%s],lib: [%s]", VER, iot.ver);
    iot.pub_msg(msg);

    sprintf(msg, "ver #2: WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d]",
            USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "maintainance") == 0)
  {
    sprintf(msg, "Maintainance: forced No-sleep [%d minutes] started", NO_SLEEP_TIME);
    start_maintainance();
    iot.pub_msg(msg);
    iot.pub_tele(msg);
  }
  else if (strcmp(incoming_msg, "sleep_p") == 0)
  {
    sprintf(msg, "sleep_Parameters: SleepDuration[%d minuntes], forcedWakeTime[%d sec], forced No-sleep [%d minutes]",
            SLEEP_TIME, FORCE_AWAKE_TIME, NO_SLEEP_TIME);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help") == 0)
  {
    sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
    iot.pub_msg(msg);
    sprintf(msg, "Help: Commands #2 - [sleep_p, sleepNow, maintainance(onWake Topic)]");

    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "sleepNow") == 0)
  {
    b4sleep();
    go2sleep.sleepNOW(SLEEP_TIME * 60);
  }
}

// +++++++++++ IFTT  ++++++++++++++
const char *server = "maker.ifttt.com";
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";

template <typename T1, typename T2, typename T3>
void makeIFTTTRequest(T1 val1, T2 val2, T3 val3)
{
  Serial.print("Connecting to ");
  Serial.print(server);

  WiFiClient client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0))
  {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected())
  {
    Serial.println("Failed to connect...");
  }

  Serial.print("Request resource: ");
  Serial.println(resource);

  String jsonObject = String("{\"value1\":\"") + val1 + "\",\"value2\":\"" + val2 + "\",\"value3\":\"" + val3 + "\"}";

  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0))
  {
    delay(100);
  }
  if (!!!client.available())
  {
    Serial.println("No response...");
  }
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("\nclosing connection");
  client.stop();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  startIOT_services();
#if USE_SLEEP
  startSleep_services();
#endif

  char a[50];
  sprintf(a, "Boot: [#%d]", bootCounter);
#if USE_VMEASURE
  bat_measure();
  char b[30];
  sprintf(b, " ,Bat measured Voltage[%.2fv]", bat_volt);
  strcat(a, b);

#endif
  Serial.println("just before first msg");
  iot.pub_msg(a);
  Serial.println("just after first msg");

  // makeIFTTTRequest(go2sleep.WakeStatus.name, a, "The-End");
  // iot.pub_tele(a);
}

void loop()
{
  // static int loopc=0;
  // Serial.print("LOOOP: ");
  // Serial.println(loopc);
  // loopc++;
  iot.looper();
#if USE_SLEEP
  go2sleep.wait_forSleep(iot.networkOK, no_sleep_flag);
#endif
  delay(100);
}