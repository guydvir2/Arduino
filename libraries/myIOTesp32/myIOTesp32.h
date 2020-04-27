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
    
    myIOT32(char *devTopic="no-name", char *ssid=SSID_ID, char *wifi_p=PASS_WIFI, char *mqtt_broker=MQTT_SERVER1, char *mqttU=MQTT_USER, char *mqttP=MQTT_PASS, int port = 1883);
    void looper();
    void start();

};

#endif