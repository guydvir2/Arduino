#include <ArduinoOTA.h>

// ~~~~~~ Scrpit Services ~~~~~~
#define USE_WIFI true
#define USE_OTA false
#define USE_IFTTT true
#define USE_SLEEP true
#define USE_DHT false
#define USE_LCD false
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DEEPSLEEP_TIME 60
#define FORCED_WAKE_TIME 10 // seconds till sleep
#define DEV_NAME "ESP32lite"

// ~~~~~~~~~ WiFi ~~~~~~~~~~~~~~~~
#include "WiFi.h"
const char *ssid = "Xiaomi_D6C8";
const char *password = "guyd5161";

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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~ NTP ~~~~~~~~~~~~~~~~~~~~~
#include "time.h"
const int gmtOffset_sec = 2 * 3600;
const int daylightOffset_sec = 0; //3600;
struct tm timeinfo;
time_t epoch_time;
char clock_char[20];
char date_char[20];

void startNTP()
{
  const char *ntpServer = "pool.ntp.org";
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
bool getTime()
{
  byte a;
  while (a <= 2)
  {
    if (getLocalTime(&timeinfo))
    {
      time(&epoch_time);
      return 1;
    }
    delay(50);
    a++;
  }
  return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~
#include <PubSubClient.h>
const char *mqtt_server = "192.168.3.200";
bool firstmsg = true;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char *MQTT_group = "myHome/TESTS/";
const char *MQTT_deviceName = DEV_NAME;
const char *MQTT_publishMSG = "myHome/Messages";
const char *MQTT_publishLOG = "myHome/log";

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
}
void mqttReconnect()
{
  long loopclock = millis();
  int sec_retry_reconnect = 20;

  while (!mqttClient.connected() && millis() - loopclock <= sec_retry_reconnect * 1000)
  {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(MQTT_deviceName))
    {
      Serial.println("connected");
      mqttClient.subscribe("myHome/TESTS/esp32");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void mqtt_pubmsg(char *msg)
{
  char t[150];
  getLocalTime(&timeinfo);
  sprintf(t, "[%04d-%02d-%02d %02d:%02d:%02d] [%s%s] %s ",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
          MQTT_group, MQTT_deviceName, msg);
  mqttClient.publish(MQTT_publishMSG, t);
}

void mqtt_publog(char *msg)
{
  char t[150];
  getLocalTime(&timeinfo);
  sprintf(t, "[%04d-%02d-%02d %02d:%02d:%02d] [%s%s] %s ",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
          MQTT_group, MQTT_deviceName, msg);
  mqttClient.publish(MQTT_publishLOG, t);
}

void mqtt_loop()
{
  if (!mqttClient.connected())
  {
    mqttReconnect();
  }
  else
  {
    mqttClient.loop();
  }
  if (firstmsg)
  {
    // char tmsg[30];
    // sprintf(tmsg, " << BOOT >>, Awake for [%d] sec", TIME_AWAKE);
    mqtt_publog("<< BOOT >>");
    firstmsg = false;
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~ OTA ~~~~~~~~~~~~~~~~~~~
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#define OTA_TIME 10 // minutes

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
  if (millis() < 1000 * 60 * OTA_TIME)
  {
    ArduinoOTA.handle();
  }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ Temp & Humid Sensor ~~~~~~~
#if USE_DHT
#include "DHT.h"
#define DHTPIN 14     // Digital pin connected to the DHT sens
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

float h = 0;
float t = 0;
long lastDHTRead = 0;

void startDHT()
{
  dht.begin();
}

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
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~ LCD Display ~~~~~~~~~~
#if USE_LCD
#include <LiquidCrystal_I2C.h>
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void startLCD()
{
  lcd.init();
  lcd.backlight();
}
void clock_update()
{
  getLocalTime(&timeinfo);
  sprintf(clock_char, "%02d:%02d:%02d     %.0fC", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, t);
  sprintf(date_char, "%04d-%02d-%02d   %.0f%%", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, h);
}
void update_clock_lcd()
{
  clock_update();
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(clock_char);
  lcd.setCursor(0, 1);
  lcd.print(date_char);
}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~ DAC & Solar Panel ~~~~~~~~~~~
struct voltReader
{
  int pin;
  float ADC_value;
  float v_divider;
  float correctF;
  float calc_value;
  float vlogic;
  float ADC_res;
};

voltReader battery = {39, 0.0, 0.5, 1.15, 0, 3.3, 4096};
voltReader solarPanel = {36, 0.0, 1 / 3, 1.1, 0};
const int Vsamples = 10;
float batADC_atBoot;

void Vmeasure()
{
  battery.ADC_value = 0.0;
  solarPanel.ADC_value = 0.0;
  for (int a = 0; a < Vsamples; a++)
  {
    battery.ADC_value += analogRead(battery.pin);
    solarPanel.ADC_value += analogRead(solarPanel.pin);
    delay(50);
  }
  battery.ADC_value /= (float)Vsamples;
  battery.calc_value = battery.ADC_value / battery.ADC_res * battery.vlogic * battery.correctF / battery.v_divider;
  solarPanel.ADC_value /= (float)Vsamples;
  solarPanel.calc_value = solarPanel.ADC_value / solarPanel.ADC_res * solarPanel.vlogic * solarPanel.correctF / solarPanel.v_divider;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~ Sleep ~~~~~~~~~~~~~~
#if USE_SLEEP
#define uS_TO_S_FACTOR 1000000ULL    /* Conversion micro seconds to seconds */
#define TIME_TO_SLEEP DEEPSLEEP_TIME /* minutes in deep sleep */
#define TIME_AWAKE FORCED_WAKE_TIME  /* Seconds until deep sleep */
RTC_DATA_ATTR long lastsleeptime = 0;
RTC_DATA_ATTR long calc_waketime = 0;
char sleepstr[150];

void sleepNOW(int sec2sleep = 2700)
{
  char tmsg[30];
  sprintf(tmsg, "Going to DeepSleep for [%d] sec", sec2sleep);
  Serial.println(tmsg);
  // mqtt_pubmsg(tmsg);
  Serial.flush();
  esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void printClock()
{
  Serial.print(timeinfo.tm_hour);
  Serial.print(":");
  Serial.print(timeinfo.tm_min);
  Serial.print(":");
  Serial.print(timeinfo.tm_sec);
  Serial.println("");
}

bool check_awake_ontime()
{
  delay(2500);
  getTime();
  sprintf(sleepstr, "deviceName:[%s]; Wake: [%02d:%02d:%02d]; ", DEV_NAME, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  if (timeinfo.tm_year >= 120) // year 2020
  {
    if (lastsleeptime > 0)
    { // not first boot
      long current_boottime = epoch_time;
      int t_delta = epoch_time - calc_waketime;

      char tt[20];
      sprintf(tt, "t_delta: %d; ", t_delta);
      strcat(sleepstr, tt);

      if (t_delta >= 0)
      {
        Serial.println("OK - WOKE UP after due time: ");
        return 1;
      }
      else
      {
        Serial.println("FAIL- woke up before time: ");
        sleepNOW(-1 * t_delta);
        return 0;
      }
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 0;
  }
}

void lowbat_sleep(int vbat = 1800)
{
  Vmeasure();
  Serial.print("battery value is: ");
  Serial.println(battery.ADC_value);
  if (battery.ADC_value < vbat)
  {
    sleepNOW();
  }
}
long calc_sleepTime()
{
  getTime();
  float correctionFactor = 1.0070;
  long clockCount = correctionFactor * (TIME_TO_SLEEP * 60 - (timeinfo.tm_min * 60 + timeinfo.tm_sec) % (TIME_TO_SLEEP * 60));
  lastsleeptime = epoch_time;
  calc_waketime = epoch_time + clockCount;

  char tt[100];
  sprintf(tt, "wakeDuration: [%.2fs]; startSleep: [%02d:%02d:%02d]; sleepDuration: [%ds]; driftFactor: [%.0fs]",
          (float)millis() / 1000.0, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, clockCount, clockCount * (1 - 1 / correctionFactor));
  strcat(sleepstr, tt);

  return clockCount;
}
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// +++++++++++ IFTT  ++++++++++++++
#if USE_IFTTT
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";
const char *server = "maker.ifttt.com";
#define UPLOAD_INTERVAL 60 * 15 // Seconds to upload IFTTT
long lastUPLOAD = 0;
bool firstUpload = true;

void makeIFTTTRequest(float val1, float val2, char *val3)
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
#endif
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Boot ~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Start Main ~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup()
{
  Serial.begin(9600);
  Serial.printf("Connecting to %s ", ssid);
  pinMode(battery.pin, INPUT);
  pinMode(solarPanel.pin, INPUT);

  // lowbat_sleep(1750);
  batADC_atBoot = battery.ADC_value;

#if USE_DHT
  startDHT();
  getDHTreading();
#endif

#if USE_LCD
  startLCD();
#endif

#if USE_WIFI
  if (startWifi())
  {
    mqttConnect();
    startNTP();
#if USE_SLEEP
    check_awake_ontime();
#endif
  }
#if USE_OTA
  startOTA();
#endif
#endif
}

void loop()
{
#if USE_DHT
  getDHTreading();
#endif

#if USE_WIFI
#if USE_IFTTT
  if (firstUpload)
  {
    Vmeasure();
#if USE_DHT
    char str[150];
    sprintf(str, "T=%.1fC;H=%.0f%%; ADC_bat_boot=%.0f", t, h, batADC_atBoot);
    makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, str);
// #else
//       makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, "NONE");
#endif
  }
#endif
  mqtt_loop();
#endif

#if USE_SLEEP
  if (millis() >= TIME_AWAKE * 1000)
  {
    long a = calc_sleepTime();
    makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, sleepstr);
    sleepNOW(a);
  }
#endif

#if USE_LCD
  update_clock_lcd();
#endif
#if USE_OTA
  OTAlooper();
#endif
  delay(100);
}
