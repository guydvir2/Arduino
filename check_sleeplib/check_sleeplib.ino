#include <myESP32sleep.h>
#include <myIOTesp32.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define VER "ESP32_0.1v"
// ~~~~~~~ myIOT32 ~~~~~~~~
#define DEVICE_TOPIC "ESP32_bad"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "solarPower"
#define MQTT_TELEGRAM "myHome/Telegram"
#define MQTT_EXT_TOPIC MQTT_PREFIX "/" MQTT_GROUP "/" DEVICE_TOPIC "/" \
                                   "onWake"
#define USE_SERIAL true
#define USE_OTA true
#define USE_WDT false
#define USE_EXT_TOPIC true
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
void ext_MQTT(char *incoming_msg)
{
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Sleep ~~~~~~~~~~~
#define SLEEP_TIME 30
#define FORCE_AWAKE_TIME 30
#define NO_SLEEP_TIME 4
#define DEV_NAME "ESP32"

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);

void b4sleep()
{
  Serial.println("Going to Sleep");
  Serial.flush();
}
void startSleep_services()
{
  go2sleep.run_func(b4sleep); // define a function to be run prior to sleep.
  go2sleep.no_sleep_minutes = NO_SLEEP_TIME;
  go2sleep.startServices();
}

#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "Xiaomi_D6C8";
const char *password = "guyd5161";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void startWiFi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("myHome/Messages", "esp32 Reconnect");
      // ... and resubscribe
      mqttClient.subscribe("myHome/ESP32");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void setup()
{
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  // Serial.begin(9600);
  // Serial.println("\nBegin");
  // Serial.println(bootCounter);
  // startIOT_services();

  Serial.begin(9600);
  startWiFi();
  mqttClient.setServer("192.168.3.200", 1883);
  mqttClient.setCallback(callback);

  startSleep_services();
  reconnect();
  char a[50];

  sprintf(a, "boot: %d", go2sleep.WakeStatus.bootCount);
  mqttClient.publish("myHome/Messages", a);
}

void loop()
{
  if (!mqttClient.connected())
  {
    reconnect();
  }
  mqttClient.loop();
  // iot.looper();
  go2sleep.wait_forSleep();
  delay(100);
}
