#include <Arduino.h>
#include <myIOT.h>

extern myIOT iot;

// ~~~~ TO & dailyTO ~~~~~~
mySwitch TOswitch;
const int START_dailyTO[] = {18, 0, 0};
const int END_dailyTO[] = {18, 30, 0};
const int safetyOff = 180;
const int TimeOut = 15;
const char *clockAlias = "Daily TimeOut";

void TOswitch_init()
{
    TOswitch.useSerial = USE_SERIAL;
    TOswitch.useInput = false; /* using input not from mySwitch*/
    TOswitch.useEXTtrigger = false;
    TOswitch.useHardReboot = false;
    TOswitch.badBoot = USE_RESETKEEPER; /* ResetKeeper */
    TOswitch.useDailyTO = false;        /* will be used from HomeAsistant */
    TOswitch.usesafetyOff = true;
    TOswitch.set_safetyoff = safetyOff;
    TOswitch.usequickON = false;
    TOswitch.onAt_boot = false;
    TOswitch.usetimeOUT = true;   /* although defined - most cases uses ad-hoc timeouts*/
    TOswitch.is_momentery = true; /* using input not from mySwitch*/
    TOswitch.inputState = HIGH;   /* using input not from mySwitch*/
    TOswitch.inputPin = INPUT1;   /* using input not from mySwitch*/

    TOswitch.config(RELAY1, TimeOut, "myBoiler");
}
void TOswitch_begin()
{
    // Start Services after After Wifi is On and have a valid clock
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
