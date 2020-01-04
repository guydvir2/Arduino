#include "WiFi.h"
#include <PubSubClient.h>
#include "time.h"
#include <Arduino.h>
// #include <TimeLib.h>
// #include <Time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char *ssid = "Xiaomi_D6C8";
const char *password = "guyd5161";
const char *ntpServer = "pool.ntp.org";
const char *mqtt_server = "192.168.3.200";

struct tm timeinfo;
const int gmtOffset_sec = 2 * 3600;
const int daylightOffset_sec = 0; //3600;
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define TIME_TO_SLEEP 60 * 60
#define TIME_AWAKE 15

// +++++++++++ IFTT  ++++++++++++++
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";
const char *server = "maker.ifttt.com";

void makeIFTTTRequest(float val1, float val2, float val3)
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

bool startWifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && millis() < 30000)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(3600);
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println(WiFi.localIP());
    return 1;
  }
  else
  {
    Serial.println("Failed connect to wifi");
    return 0;
  }
}

void getTime()
{
  getLocalTime(&timeinfo);
}

void mqttConnect()
{
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void mqttCallback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  // if (String(topic) == "esp32/output")
  // {
  //   Serial.print("Changing output to ");
  //   if (messageTemp == "on")
  //   {
  //     Serial.println("on");
  //     digitalWrite(ledPin, HIGH);
  //   }
  //   else if (messageTemp == "off")
  //   {
  //     Serial.println("off");
  //     digitalWrite(ledPin, LOW);
  //   }
  // }
}

void mqttReconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("ESP32"))
    {
      Serial.println("connected");
      // Subscribe
      mqttClient.subscribe("myHome/TESTS/esp32");
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

void mqtt_pubmsg(char *msg)
{
  mqttClient.publish("myHome/Messages", msg);
}

void setup()
{
  Serial.begin(9600);
  Serial.printf("Connecting to %s ", ssid);
  startWifi();
  mqttConnect();
  getTime();

  makeIFTTTRequest(1.13, 1.1, 13);
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // mqtt_pubmsg("goto sleep");
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
  {
    mqttReconnect();
  }
  mqttClient.loop();
  if (millis() >= TIME_AWAKE * 1000)
  {
    Serial.println("Going to sleep now");
    mqtt_pubmsg("goto sleep");
    Serial.flush();
    esp_sleep_enable_timer_wakeup((TIME_TO_SLEEP - TIME_AWAKE) * uS_TO_S_FACTOR);

    esp_deep_sleep_start();
  }
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  delay(100);
}
