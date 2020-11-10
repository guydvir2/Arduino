#include <Arduino.h>
#include <myIOT.h>

extern myIOT iot;

// ~~~~ TO & dailyTO ~~~~~~
mySwitch TOswitch;
const int START_dailyTO[] = {18, 0, 0};
const int END_dailyTO[] = {18, 30, 0};
const int safetyOff = 180;
const int TimeOut = 90 ; 
const char *clockAlias = "Daily TimeOut";

void TOswitch_init()
{
    TOswitch.useSerial = true;
    TOswitch.useInput = false;
    TOswitch.useEXTtrigger = false;
    TOswitch.useHardReboot = false;
    TOswitch.is_momentery = true;
    TOswitch.badBoot = true;
    TOswitch.useDailyTO = true;
    TOswitch.usesafetyOff = true;
    TOswitch.set_safetyoff = safetyOff;
    TOswitch.usequickON = false;
    TOswitch.onAt_boot = false;
    TOswitch.usetimeOUT = false;
    TOswitch.inputState = HIGH;
    TOswitch.inputPin = INPUT1;

    TOswitch.config(RELAY1, TimeOut, "Boiler");
}
void startdTO()
{
    // After Wifi is On
    if (TOswitch.useDailyTO)
    {
        TOswitch.setdailyTO(START_dailyTO, END_dailyTO);
    }
    TOswitch.begin();
}
void TOswitch_looper()
{
    char msgtoMQTT[150];
    byte mtyp;

    TOswitch.looper(iot.mqtt_detect_reset);
    if (TOswitch.postMessages(msgtoMQTT, mtyp))
    {
        if (mtyp == 0)
        {
            iot.pub_msg(msgtoMQTT);
        }
        if (mtyp == 1)
        {
            iot.pub_log(msgtoMQTT);
        }
        if (mtyp == 2)
        {
            iot.pub_state(msgtoMQTT);
        }
    }
}

// ***********************************


