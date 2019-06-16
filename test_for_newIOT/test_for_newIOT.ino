/*
   Upload using Generic ESP8266
   Change default 512 kB ram to
   <<<< 1024 kB of ram !!! MUST!! >>>>
   otherwise OTA will not be loaded next time
 */

#include <myIOT.h>
#include <TimeLib.h>
#include <Arduino.h>

//##############  User Input ##################
#define DEVICE_TOPIC "HomePi/Dvir/Lights/sono"
#define VER "SonoffBasic_2.2"

//~~~Services~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_BOUNCE_DEBUG false
#define USE_EXT_BUTTONS  false
#define USE_FAT          true // Flash Assist using JSON and FS


// ~~~~~~~~~~~~~~Start services ~~~~~~~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
FVars var1("Key1");
FVars var2("key2");
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() {

        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.start_services(ADD_MQTT_FUNC);

        // var1.remove();
        // var2.setValue("GUYdvir");
        // var2.printFile();
        // var1.format();
        iot.pub_err("guy");
}

// ~~~~~~~~~~~~~~~ MQTT  ~~~~~~~~~~~~~~~~~~
void addiotnalMQTT(char *incoming_msg) {
        char state[25];
        char msg[200];
        char tempstr[50];
        char tempstr2[50];

        if (strcmp(incoming_msg, "status") == 0) {
                // relays state
                strcpy(msg, "Status: ");
                // for (int i = 0; i < NUM_SWITCHES; i++) {
                //         sprintf(tempstr, "Switch#[%d] [%s], timeOut[%s] ", i, digitalRead(relays[i]) ? "On" : "Off", inTimeOut[i] ? "Yes" : "No");
                //         strcat(msg, tempstr);
                // }
                iot.pub_msg(msg);
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() {
        iot.looper(); // check wifi, mqtt, wdt
        delay(100);
}
