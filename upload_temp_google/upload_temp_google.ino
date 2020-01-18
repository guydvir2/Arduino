#include <myIOT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"
#define USE_NOTIFY_TELE false

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true
#define USE_WDT true
#define USE_OTA true
#define USE_RESETKEEPER false
#define USE_FAILNTP true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "DC_Measure"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "TESTS"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

// ~~~~~ Wifi Google Certificates ~~~~~~~~~
WiFiClientSecure client;
const char *host = "script.google.com";
const int httpsPort = 443;
const char *fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
// String google_ID = "AKfycbwV7RGU3rEBcXIfCyKZpFCHUlhEWbyB19kd9WfLtCrF49Napbs"; // TEMPS
String google_ID = "AKfycbykLwmZvUUaPq2duFYC3dvEyfTnpcvKdX9YUuyJZY9IBLfhV3c"; //TESTS

// ~~~~~~~~~ DHT Sensor ~~~~~~~~~~~~
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float humidity, temperature, heatIndex;
char str_humidity[10], str_temperature[10], str_heatIndex[10];

// ~~~~~~~~~~ Time intervals ~~~~~~~~~~
const int sleepSeconds = 60*1;
const int stayawakeSeconds = 5;
const int uploadintervalSeconds = 20;
unsigned long lastupload = 0;

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

void startDHT()
{
  // Serial.println("Initialize DHT sensor");
  dht.begin();
  delay(2000);
}

void get_setsorData()
{
  Serial.println("Read DHT sensor");
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);
}

void print_sensorData()
{
  // Convert the floats to strings and round to 2 decimal places
  dtostrf(humidity, 1, 2, str_humidity);
  dtostrf(temperature, 1, 2, str_temperature);
  dtostrf(heatIndex, 1, 2, str_heatIndex);

  Serial.printf("Humidity:    %s %%\nTemperature: %s *C\nHeat index:  %s *C\n", str_humidity, str_temperature, str_heatIndex);
}

void gotoSleep(int secs)
{
  Serial.printf("Sleep for %d seconds\n\n", secs);
  ESP.deepSleep(secs * 1000000);
  delay(10);
}

void sendData(float x, float y, int pres = 1)
{
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }
  if (client.verify(fingerprint, host))
  {
    Serial.println("certificate matches");
  }
  else
  {
    Serial.println("certificate doesn't match");
  }

  String string_x = String(x, pres);
  String string_y = String(y, pres);
  String url = "/macros/s/" + google_ID + "/exec?temperature=" + string_x + "&humidity=" + string_y;

  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\""))
  {
    Serial.println("esp8266/Arduino CI successfull!");
  }
  else
  {
    Serial.println("esp8266/Arduino CI has failed");
  }

  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}

void addiotnalMQTT(char incoming_msg[50])
{
  char state[5];
  char state2[5];
  char msg[100];
  char msg2[100];

  if (strcmp(incoming_msg, "status") == 0)
  {
    iot.pub_msg("empty_Status");
  }
  else if (strcmp(incoming_msg, "ver") == 0)
  {
    sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d],ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
    iot.pub_msg(msg);
  }
}

void setup()
{
  startIOTservices();
  client.setInsecure();

  Serial.println("\n\nWake up");
  pinMode(D0, WAKEUP_PULLUP);

  // startDHT();
  // get_setsorData();
  // print_sensorData();

  sendData(temperature, humidity, 2);
  lastupload = millis();
}

void loop()
{
  iot.looper();

  if (millis() >= stayawakeSeconds * 1000)
  {
    gotoSleep(sleepSeconds);
  }

  // if (millis()-lastupload >= uploadintervalSeconds * 1000)
  // {
  //   get_setsorData();
  //   sendData(temperature, humidity, 2);
  //   lastupload = millis();
  // }

  delay(100);
}
