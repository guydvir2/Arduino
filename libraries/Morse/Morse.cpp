// \\*
//   Morse.cpp - Library for flashing Morse code.
//   Created by David A. Mellis, November 2, 2007.
//   Released into the public domain.
// */

// #include "Arduino.h"
#include "Morse.h"
#include "ESP8266WiFi.h"


Morse::Morse(const char *ssid, const char *pwd)
{
  WiFiClient espClient;
  _SSID = ssid;
  _PWD = pwd;
  // pinMode(pin, OUTPUT);
  // _pin = pin;
  // startNetwork();


<<<<<<< HEAD
// hold informamtion
char msg[150];
char timeStamp[50];
char bootTime[50];
bool firstRun = true;
// ###################
int _pin;
=======
}
void Morse::startNetwork() {
        long startWifiConnection = millis();

>>>>>>> d503db23c0f8626d760da71ff1ac5091d2968880

        WiFi.mode(WIFI_STA);
        WiFi.begin("123", "123");
        WiFi.setAutoReconnect(true);

        // in case of reboot - timeOUT to wifi
        while (WiFi.status() != WL_CONNECTED){
                delay(500);
        }


}
