/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <TZ.h>
#define TIME_ZONE TZ_Asia_Jerusalem

#ifndef STASSID
#define STASSID "Xiaomi_D6C8"
#define STAPSK  "guyd5161"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;



void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  configTime(TIME_ZONE, "pool.ntp.org");
  time_t now = time(nullptr); // before getting cloick
  while (now < 1627735850) {
    delay(20);
    Serial.print(".");
    now = time(nullptr);
  }
  struct tm *tm = localtime(&now);
  Serial.println(tm->tm_year+1900);
}

void loop() {
  delay(5);
}
