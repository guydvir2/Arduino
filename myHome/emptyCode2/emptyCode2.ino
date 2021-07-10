#include <myIOT2.h>
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters

#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

void show_services()
{
        static bool showed = false;
        if (!showed)
        {
                char msg[300];
                sprintf(msg, "Service: useSerial[%d], useWDT[%d], useOTA[%d], useResetKeeper[%d], useextTopic[%d], resetFailNTP[%d], useDebug[%d], debug_level[%d], useNetworkReset[%d], noNetwork_reset[%d],useBootClockLog[%d]",
                        iot.useSerial, iot.useWDT, iot.useOTA, iot.useResetKeeper, iot.useextTopic, iot.resetFailNTP, iot.useDebug, iot.debug_level, iot.useNetworkReset, iot.noNetwork_reset, iot.useBootClockLog);
                Serial.println(msg);
                showed = true;
        }
}

void listDir(const char *dirname)
{
        Serial.printf("Listing directory: %s\n", dirname);

        Dir root = LittleFS.openDir(dirname);

        while (root.next())
        {
                File file = root.openFile("r");
                Serial.print("  FILE: ");
                Serial.print(root.fileName());
                Serial.print("  SIZE: ");
                Serial.print(file.size());
                //     time_t cr = file.getCreationTime();
                //     time_t lw = file.getLastWrite();
                //     file.close();
                //     struct tm * tmstruct = localtime(&cr);
                //     Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
                //     tmstruct = localtime(&lw);
                //     Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        }
}

void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
#endif
        Serial.println("BOOT!!!!");
        LittleFS.begin();
        listDir("/");
}
void loop()
{
        iot.looper();
        show_services();
        delay(100);
}
