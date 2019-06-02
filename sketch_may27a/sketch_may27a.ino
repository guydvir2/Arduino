#include <ESP8266WiFi.h>
//#include <Time.h>
//#include <TimeLib.h>
#include <NtpClientLib.h>

char *ssid = "Xiaomi_D6C8";
char *password = "guyd5161";

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  NTP.begin("pool.ntp.org", 2, true);
  NTP.setInterval(12,1800);
  delay(500);
  time_t t = now();

  Serial.print(hour(t));
  Serial.print(":");
  Serial.print(minute(t));
  Serial.print(":");
  Serial.println(second(t));

  Serial.println(timeStatus());
  Serial.println(NTP.getTimeDateString ());
  
  
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
