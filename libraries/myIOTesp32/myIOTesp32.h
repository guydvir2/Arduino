#ifndef myIOT32_h
#define myIOT32_h
#include "Arduino.h"

// OTA libraries
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "secrets.h"
#include "EspMQTTClient.h"


class myIOT32
{
private:
    void onConnectionEstablished();

public:
    EspMQTTClient client;
    
    myIOT32(char *ssid, char *wifi_p, char *mqtt_broker, char *mqttU, char *mqttP, char *devTopic, int port = 1883);
    void looper();

};

#endif