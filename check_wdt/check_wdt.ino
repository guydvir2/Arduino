//#include <ESP8266WiFi.h>
//#include <TimeLib.h>
//#include <NtpClientLib.h>
//#include <PubSubClient.h> //MQTT
#include <Ticker.h> //WDT


//wifi creadentials
const char* ssid;
const char* ssid_0 = "HomeNetwork_2.4G";
const char* ssid_1 = "Xiaomi_D6C8";
const char* password = "guyd5161";
//###################################

int wdtResetCounter = 0;
int wdtMaxRetries = 2;
int timeAdd=100;
Ticker wdt;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(50);
  Serial.println("HI");
  wdt.attach(1, feedTheDog);

}

void sendReset(char *header) {
  char temp[150];

  sprintf(temp, "[%s] - Reset sent", header);
//  pub_msg(temp);
  delay(100);
//  ESP.restart();
ESP.reset();
}

void feedTheDog() {
  wdtResetCounter++;
  if (wdtResetCounter >= wdtMaxRetries) {
    sendReset("WatchDog");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  wdtResetCounter = 0;
  Serial.print(wdtResetCounter);
  delay(1000+timeAdd);
  timeAdd=timeAdd*1.5;
  Serial.print(timeAdd);
}
