#include <ArduinoOTA.h>

// ~~~~~~ Scrpit Services ~~~~~~
#define USE_WIFI true
#define USE_OTA false
#define USE_IFTTT true
#define USE_SLEEP true
#define USE_VMEASURE false
#define USE_DHT false
#define USE_LCD false
#define USE_EEPROM true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DEEPSLEEP_TIME 60
#define FORCED_WAKE_TIME 15 // seconds till sleep
#define DEV_NAME "ESP32lite"

// ~~~~~~~~~ WiFi ~~~~~~~~~~~~~~~~
#include "WiFi.h"
const char *ssid = "Xiaomi_D6C8";
const char *password = "guyd5161";

bool startWifi()
{
#if USE_WIFI
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
#endif
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
#if USE_WIFI
  const char *ntpServer = "pool.ntp.org";
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif
}
bool getTime()
{
#if USE_WIFI
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
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~
#include <PubSubClient.h>
const char *mqtt_server = "192.168.3.200";
bool firstmsg = true;
#if USE_WIFI
WiFiClient espClient;
PubSubClient mqttClient(espClient);
#endif

const char *MQTT_group = "myHome/TESTS/";
const char *MQTT_deviceName = DEV_NAME;
const char *MQTT_publishMSG = "myHome/Messages";
const char *MQTT_publishLOG = "myHome/log";

void mqttConnect()
{
#if USE_WIFI
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
#endif
}
void mqttCallback(char *topic, byte *message, unsigned int length)
{
#if USE_WIFI
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
#endif
}
void mqttReconnect()
{
#if USE_WIFI
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
#endif
}
void mqtt_pubmsg(char *msg)
{
#if USE_WIFI
  char t[150];
  getLocalTime(&timeinfo);
  sprintf(t, "[%04d-%02d-%02d %02d:%02d:%02d] [%s%s] %s ",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
          MQTT_group, MQTT_deviceName, msg);
  mqttClient.publish(MQTT_publishMSG, t);
#endif
}

void mqtt_publog(char *msg)
{
#if USE_WIFI
  char t[150];
  getLocalTime(&timeinfo);
  sprintf(t, "[%04d-%02d-%02d %02d:%02d:%02d] [%s%s] %s ",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
          MQTT_group, MQTT_deviceName, msg);
  mqttClient.publish(MQTT_publishLOG, t);
#endif
}

void mqtt_loop()
{
#if USE_WIFI
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
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~ OTA ~~~~~~~~~~~~~~~~~~~
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#define OTA_TIME 10 // minutes

void startOTA()
{
#if USE_OTA
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
#endif
}
void OTAlooper()
{
#if USE_OTA
  if (millis() < 1000 * 60 * OTA_TIME)
  {
    ArduinoOTA.handle();
  }
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ Temp & Humid Sensor ~~~~~~~
#include "DHT.h"
#define DHTPIN 14     // Digital pin connected to the DHT sens
#define DHTTYPE DHT11 // DHT 11
#if USE_DHT
DHT dht(DHTPIN, DHTTYPE);
#endif

float h = 0;
float t = 0;
long lastDHTRead = 0;

void startDHT()
{
#if USE_DHT
  dht.begin();
#endif
}

void getDHTreading()
{
#if USE_DHT
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
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~ LCD Display ~~~~~~~~~~
#include <LiquidCrystal_I2C.h>
int lcdColumns = 16;
int lcdRows = 2;

#if USE_LCD
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
#endif

void startLCD()
{
#if USE_LCD
  lcd.init();
  lcd.backlight();
#endif
}
void clock_update()
{
#if USE_LCD
  getLocalTime(&timeinfo);
  sprintf(clock_char, "%02d:%02d:%02d     %.0fC", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, t);
  sprintf(date_char, "%04d-%02d-%02d   %.0f%%", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, h);
#endif
}
void update_clock_lcd()
{
#if USE_LCD
  clock_update();
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(clock_char);
  lcd.setCursor(0, 1);
  lcd.print(date_char);
#endif
}
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
#if USE_VMEASURE
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
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~ USE EEPROM ~~~~~~~~~~~
#include "EEPROM.h"
#define EEPROM_SIZE 64

int getEEPROMvalue(byte i = 0)
{
#if USE_EEPROM
  int eeprom_drift = EEPROM.read(i) * pow(-1, EEPROM.read(i + 1));
  return eeprom_drift;
#endif
}

void saveEEPROMvalue(int val, byte i = 0)
{
#if USE_EEPROM
  EEPROM.write(i, abs(val));
  if (val < 0)
  {
    EEPROM.write(i + 1, 1);
    EEPROM.commit();
  }
  else
  {
    EEPROM.write(i + 1, 2);
    EEPROM.commit();
  }
#endif
}

void start_eeprom(byte i = 0)
{
#if USE_EEPROM
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("Fail to load EEPROM");
  }
#endif
}

bool updateDrift_EEPROM(int drift_value, byte cell = 0, byte min_clock_err = 2)
{
#if USE_EEPROM
  if (abs(drift_value) >= min_clock_err)
  {
    int savedDrift = getEEPROMvalue();
    saveEEPROMvalue(savedDrift + drift_value, cell);
    return 1;
  }
  else
  {
    return 0;
  }
#endif
}

// ~~~~~~~~~~~ Sleep ~~~~~~~~~~~~~~
#define uS_TO_S_FACTOR 1000000ULL    /* Conversion micro seconds to seconds */
#define TIME_TO_SLEEP DEEPSLEEP_TIME /* minutes in deep sleep */
#define TIME_AWAKE FORCED_WAKE_TIME  /* Seconds until deep sleep */

RTC_DATA_ATTR long clock_beforeSleep = 0;
RTC_DATA_ATTR long clock_expectedWake = 0;
RTC_DATA_ATTR int bootCounter = 0;

char sleepstr[250];

void sleepNOW(int sec2sleep = 2700)
{
#if USE_SLEEP
  char tmsg[30];

  sprintf(tmsg, "Going to DeepSleep for [%d] sec", sec2sleep);
  Serial.println(tmsg);
  Serial.println("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  // mqtt_pubmsg(tmsg);
  Serial.flush();
  esp_sleep_enable_timer_wakeup(sec2sleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
#endif
}

// void printClock()
// {
//   Serial.print(timeinfo.tm_hour);
//   Serial.print(":");
//   Serial.print(timeinfo.tm_min);
//   Serial.print(":");
//   Serial.print(timeinfo.tm_sec);
//   Serial.println("");
// }

void check_awake_ontime(int min_t_avoidSleep = 10)
{
#if USE_SLEEP
  delay(2500);
  getTime();
  bootCounter++;

  // Serial.print("WAKE CLOCK: ");
  // printClock();
  // Serial.print("WAKE epoch:");
  // Serial.println(epoch_time);
  // Serial.print("last epoch:");
  // Serial.println(clock_beforeSleep);
  sprintf(sleepstr, "deviceName:[%s]; nominalSleep: [%d min]; ForcedWakeTime: [%d sec];  Boot#: [%d]; WakeupClock: [%02d:%02d:%02d];",
          DEV_NAME, DEEPSLEEP_TIME, FORCED_WAKE_TIME, bootCounter, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  if (timeinfo.tm_year >= 120) // year 2020
  {
    if (clock_beforeSleep > 0)
    {                                                                         // not first boot
      int t_delta = epoch_time - clock_expectedWake - (int)(millis() / 1000); // diff between calc wake clock and current time

      char tt[100];
      sprintf(tt, "lastSleep: [%d sec]; drift_lastSleep: [%d sec]; ", clock_expectedWake - clock_beforeSleep, t_delta);
      Serial.println(tt);
      strcat(sleepstr, tt);

#if USE_EEPROM
      bool up = updateDrift_EEPROM(t_delta, 0);
      {
        sprintf(tt, "driftUpdate: [%s]; ", up ? "YES" : "NO");
        Serial.println(tt);
        strcat(sleepstr, tt);

        // Serial.print("drift value updated: ");
        // Serial.print(t_delta);
        // Serial.println(" sec");
      }
#endif

      if (t_delta >= 0)
      {
        Serial.println("OK - WOKE UP after due time: ");
      }
      else
      {
        Serial.println("FAIL- woke up before time: ");
        int tempSleep = epoch_time - clock_expectedWake;
        if (abs(tempSleep) < min_t_avoidSleep)
        {
          sprintf(tt, "syncPause: [%d sec]; ", abs(tempSleep));
          strcat(sleepstr, tt);
          // Serial.print("pausing ");
          // Serial.print(tempSleep);
          // Serial.println(" sec");

          delay(1000 * abs(tempSleep));
        }
        else
        {
          // Serial.println("going to temp sleep");
          sleepNOW(-1 * tempSleep);
        }
      }
    }
    else
    {
      saveEEPROMvalue(0, 0);
    }
  }
  else
  {
    Serial.println("BAD NTP");
  }
#endif
}

void lowbat_sleep(int vbat = 1800)
{
#if USE_SLEEP
#if USE_VMEASURE
  Vmeasure();
  Serial.print("battery value is: ");
  Serial.println(battery.ADC_value);
  if (battery.ADC_value < vbat)
  {
    sleepNOW();
  }
#endif
#endif
}
long calc_nominal_sleepTime()
{
#if USE_SLEEP
long nominal_nextSleep =0;

  if (getTime())
  {

    nominal_nextSleep = TIME_TO_SLEEP * 60 - (timeinfo.tm_min * 60 + timeinfo.tm_sec) % (TIME_TO_SLEEP * 60);
    clock_beforeSleep = epoch_time;                      // RTC var
    clock_expectedWake = epoch_time + nominal_nextSleep; // RTC var
  }
  else // fail to obtain clock
  {
    nominal_nextSleep = TIME_TO_SLEEP * 60;
  }

  // Serial.print("\ngoing to sleep at:");
  // Serial.println(clock_beforeSleep);
  // Serial.print("going to wake at:");
  // Serial.println(clock_expectedWake);

  char tt[100];
  sprintf(tt, "wakeDuration: [%.2fs]; startSleep: [%02d:%02d:%02d]; sleepFor: [%d sec]; drift: [%d sec]",
          (float)millis() / 1000.0, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, nominal_nextSleep, getEEPROMvalue());
  strcat(sleepstr, tt);

  return nominal_nextSleep;
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// +++++++++++ IFTT  ++++++++++++++
const char *resource = "/trigger/send_reading/with/key/cFLymB4JT9tlODsKLFn9TA";
const char *server = "maker.ifttt.com";
#define UPLOAD_INTERVAL 60 * 15 // Seconds to upload IFTTT
long lastUPLOAD = 0;
bool firstUpload = true;

void makeIFTTTRequest(float val1, float val2, char *val3)
{
#if USE_IFTTT
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
#endif
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Boot ~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Start Main ~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup()
{
  Serial.begin(9600);
  Serial.println("\n±±±±±±±±±±± START ±±±±±±±±±±±±±±");
  Serial.printf("Connecting to %s ", ssid);
  pinMode(battery.pin, INPUT);
  pinMode(solarPanel.pin, INPUT);

  // lowbat_sleep(1750);
  batADC_atBoot = battery.ADC_value;

  startDHT();
  getDHTreading();
  startLCD();
  start_eeprom();

  if (startWifi())
  {
    startNTP();
    mqttConnect();
    check_awake_ontime();
    startOTA();
  }
}

void loop()
{
  getDHTreading();

  // if (firstUpload)
  // {
  //   Vmeasure();
  //   char str[150];
  //   sprintf(str, "T=%.1fC;H=%.0f%%; ADC_bat_boot=%.0f", t, h, batADC_atBoot);
  //   makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, str);
  //   // #else
  //   //       makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, "NONE");
  // }
  mqtt_loop();

  if (millis() >= TIME_AWAKE * 1000)
  {
    int a = calc_nominal_sleepTime();
    int b = getEEPROMvalue();

    Serial.println("~~~~~~~~ SLEEP CALC ~~~~~~~~");
    Serial.print("Nominal Sleep: ");
    Serial.println(a);
    Serial.print("drift is: ");
    Serial.println(b);
    Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    makeIFTTTRequest(battery.ADC_value, solarPanel.ADC_value, sleepstr);

    sleepNOW(a - b);
  }

  update_clock_lcd();
  OTAlooper();
  delay(100);
}
