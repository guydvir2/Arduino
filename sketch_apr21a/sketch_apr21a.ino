#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <ArduinoJson.h>

#define CHAT_ID "596123373"

// Initialize Wifi connection to the router
char ssid[] = "National-Hotel";     // your network SSID (name)
char password[] = "abcde12345"; // your network key

// Initialize Telegram BOT
#define BOTtoken "497268459:AAESYm27tJfNXwnnnn0slbmWnkqvbWgQEyw"  // your Bot Token (Get from Botfather)

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);
void setup() {
  Serial.begin(9600);
  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(bot.sendMessage(CHAT_ID, "Did you see the action message?"));

  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
