
#include <myIOT.h>
#include <myJSON.h>
#include <Arduino.h>
#include <TimeLib.h>

// ********** Sketch Services  ***********
#define VER              "Wemos_1.0"
#define USE_INPUTS       false


// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       true
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "LeakDetector"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~ ResetKeeper Vars ~~~~~~~
bool badReboot        = false;
bool checkbadReboot   = true;
bool boot_overide     = true;

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW

bool swState [NUM_SWITCHES];
bool last_swState [NUM_SWITCHES];
bool inputs_lastState[NUM_SWITCHES];

// ~~~~ HW Pins and Statdes ~~~~
#define RELAY1          D2
#define RELAY2          5
#define INPUT1          9
#define INPUT2          3
byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};

#define timeInterval_a = 30 //seconds to be consider same operation
unsigned long startTime = 0;
unsigned long accumTime = 0;

byte inputPin = 5;


void setup() {
  pinMode(inputPin);

}

void loop() {
        iot.looper();

        delay(100);
}
