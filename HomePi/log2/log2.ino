#include <myIOT.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.0"

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "flash_logger"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "test"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

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

const char *filename = "/log.JSON";
const char *log_prefix = "log_entry"
const int JDOC_size = 300;
const int max_filesize = 1024;
const int lon_entries = 10;


const char *keys[2]={"key1","key2"};



bool readlog()
{
        File configFile = LittleFS.open(filename, "r");
        if (!configFile)
        {
                Serial.println("Failed to open config file");
                return false;
        }

        size_t size = configFile.size();
        Serial.print("file size: ");
        Serial.println(size);
        if (size > max_filesize)
        {
                Serial.println("Config file size is too large");
                return false;
        }

        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);

        StaticJsonDocument<JDOC_size> doc;
        DeserializationError error = deserializeJson(doc, buf.get());
        if (error)
        {
                Serial.println("Failed to parse config file");
                return false;
        }

        const char *serverName = doc["serverName"];
        const char *accessToken = doc["accessToken"];
        serializeJson(doc, Serial);

        Serial.print("Loaded serverName: ");
        Serial.println(serverName);
        Serial.print("Loaded accessToken: ");
        Serial.println(accessToken);
        return true;
}

void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];
        if (strcmp(incoming_msg, "status") == 0)
        {
                // sprintf(msg, "Status: Time [%s], Date [%s]", timeStamp, dateStamp);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0)
        {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0)
        {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
        }
}

void setup()
{
        startIOTservices();
}
void loop()
{
        iot.looper();
        delay(100);
}
