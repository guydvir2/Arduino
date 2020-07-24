#include <myIOT.h>
#include <Arduino.h>
#include <myJSON.h>
#include <FS.h>

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
#define DEVICE_TOPIC "empty"
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

myJSON iot_config("/myIOT.JSON", true);
myJSON parameters("/config.JSON", true);
myTest testA;

void setup()
{
        // Serial.begin(9600);
        char a[30];
        startIOTservices();
        iot_config.printFile();
        // testA.funcOne<int>(1);
        // iot_config.getValue("useSerial",a);
        // Serial.println(a);
        // if (SPIFFS.begin())
        // {
        //         DynamicJsonDocument tempJDOC(300);
        //         Dir dir = SPIFFS.openDir("/");
        //         // or Dir dir = LittleFS.openDir("/data");
        //         while (dir.next())
        //         {
        //                 Serial.println(dir.fileName());
        //                 if (dir.fileSize())
        //                 {
        //                         File f = dir.openFile("r");
        //                         deserializeJson(tempJDOC,f);

        //                         // Serial.println(f.size());
        //                 }
        //         }
        // }
        // else
        // {
        //         Serial.println("NO");
        // }
        // if (iot_config.file_exists())
        // {
        //         Serial.println("file exists");
        //         iot_config.printFile();
        //         Serial.println();
        //         parameters.printFile();
        // }
        // else
        // {
        //         Serial.println("file NOT exists");
        // }

        // char *clock = iot.timeStamp;
        //         config_file.setValue("TEST_VAL",clock);
        // char *reval;
        // if (config_file.getValue("TEST_VAL",reval)){
        //         Serial.print("This is value: ");
        // }
        // else {
        //         Serial.println("value not found");
        //         char *clock = iot.timeStamp;
        //         config_file.setValue("TEST_VAL",123);
        //         int retval = 0;
        //         config_file.getValue("TEST_VAL",reval);
        //         Serial.print("RET_VAL: ");
        //         Serial.println(retval);
        //         // Serial.println(iot.timeStamp);
        // }
}
void loop()
{
        iot.looper();
        delay(100);
}
