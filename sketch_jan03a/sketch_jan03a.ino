#include "WiFi.h"
#include <PubSubClient.h>
#include "time.h"
#include <LiquidCrystal_I2C.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Arduino.h>

// ~~~~~~~~~ WiFi ~~~~~~~~~~~~~~~~
const char *ssid = "Xiaomi_D6C8";
const char *password = "guyd5161";
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~
const char *mqtt_server = "192.168.3.200";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char *MQTT_group = "myHome/TESTS/";
const char *MQTT_deviceName = "ESP32";
const char *MQTT_publishMSG = "myHome/Messages";
const char *MQTT_publishLOG = "myHome/log";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~ NTP ~~~~~~~~~~~~~~~~~~~~~
const int gmtOffset_sec = 2 * 3600;
const int daylightOffset_sec = 0; //3600;
time_t now;
struct tm timeinfo;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Sleep ~~~~~~~~~~~~~~
#define uS_TO_S_FACTOR 1000000ULL /* Conversion micro seconds to seconds */
#define TIME_TO_SLEEP 60 * 60     /* Seconds in deep sleep */
#define TIME_AWAKE 120            /* Seconds until deep sleep */
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// +++++++++++ IFTT  ++++++++++++++
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";
const char *server = "maker.ifttt.com";
#define UPLOAD_INTERVAL 60 * 15 // Seconds to upload IFTTT
long lastUPLOAD = 0;

// ~~~~~~~~~~~~~ LCD Display ~~~~~~~~~~
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ Temp & Humid Sensor ~~~~~~~
#include "DHT.h"
#define DHTPIN 13     // Digital pin connected to the DHT sens
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

float h = 0;
float t = 0;
long lastDHTRead = 0;
bool firstUpload = true;

void getDHTreading()
{
  if (millis() - lastDHTRead >= 5000)
  {
    h = dht.readHumidity();
    t = dht.readTemperature();
    lastDHTRead = millis();
    //   Serial.println(F("°C "));
    // }

    if (isnan(h) || isnan(t))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
    Serial.println(WiFi.localIP());
    return 1;
  }
  else
  {
    Serial.println("Failed connect to wifi");
    return 0;
  }
}
void startNTP()
{
  const char *ntpServer = "pool.ntp.org";
  const char *mqtt_server = "192.168.3.200";
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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

void startDHT()
{
  dht.begin();
}

void sleepNOW()
{
  Serial.println("Going to sleep now");
  mqtt_pubmsg("goto sleep");
  Serial.flush();
  esp_sleep_enable_timer_wakeup((TIME_TO_SLEEP - TIME_AWAKE) * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void startLCD()
{
  lcd.init();
  lcd.backlight();
}

void startOTA()
{
  char host_name[30];
  sprintf(host_name, "%s%s", MQTT_group, MQTT_deviceName);
  ArduinoOTA.setHostname(host_name);
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
}
void OTAlooper()
{
  if (millis() < 1000 * 60 * 10)
  {
    ArduinoOTA.handle();
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.printf("Connecting to %s ", ssid);
  if (startWifi())
  {
    mqttConnect();
    startNTP();
    // Serial.println("wifi OK");
  }
  startDHT();
  delay(100);
  // Serial.println(time(&now));

  startLCD();
  startOTA();
}

void loop()
{
  if (!mqttClient.connected())
  {
    mqttReconnect();
  }
  else
  {
    mqttClient.loop();
  }
  OTAlooper();

      char clock[20];
  char date[20];

  sprintf(clock, "%02d:%02d:%02d     %.0fC", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, t);
  sprintf(date, "%04d-%02d-%02d   %.0f%%", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, h);

  getDHTreading();

  if (millis() - lastUPLOAD >= UPLOAD_INTERVAL * 1000 || firstUpload && (h != 0 && t != 0))
  {
    makeIFTTTRequest(t, h, 1.1111);
    firstUpload = false;
    lastUPLOAD = millis();
  }

  // if (millis() >= TIME_AWAKE * 1000)
  // {
  //   sleepNOW();
  // }

  getLocalTime(&timeinfo);
  lcd.setCursor(0, 0);
  lcd.print(clock);
  lcd.setCursor(0, 1);
  lcd.print(date);
  // Serial.println(time(&now));

  delay(10);

  // lcd.clear();
}
