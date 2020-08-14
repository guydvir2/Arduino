#include <myIOT.h>
#include <Arduino.h>
#include <EEPROM.h>

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
#define DEVICE_TOPIC "ESP8266_deepSleep"
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
        iot.pub_msg("I'm Fine, thank you");
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

// ~~~~~~~~~~~~~~~ Read Battery Voltage ~~~~~~~~
const float v_div_ratio = 5.0;
const float v_logic = 3.3;
const float non_linear_factor = 0.075;
const float non_linearity_margin = 0.05;
const int adc_res = 1023;
float vbat=0;

void measure_bat_v(int x=10) {
    for (int a=0; a<x; a++) {
        vbat +=analogRead(A0);
        delay(50);
    }
    vbat /=x;
    // ~~~~ non-linear behaviour ~~~
    if (vbat >non_linearity_margin ||vbat <=1-non_linearity_margin) {
        vbat *=(1-non_linear_factor);
    }
    // ~~~~~~ END ~~~~~~~~
    vbat = (vbat/(float)adc_res)*v_div_ratio*v_logic;
    Serial.printf("Battery voltage: %.2f[v]\n", vbat);
}
void start_adc() {
    char a[50];
    pinMode(A0, INPUT);
    measure_bat_v(10);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~ DeepSleep ~~~~~~~~~~~~

#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */
#define forceWake_sec 15
#define time2Sleep 600*3
FVars FVAR_lastboot("lastBoot");
FVars FVAR_bootCounter("bootCounter");

int drift=0;

void onWake_cb() {
    char a[100];
    int bc = 0;

    FVAR_bootCounter.getValue(bc);
    sprintf(a, "BootCounter: [#%d], sleepCycle: [%d sec], drift: [%d sec], Battery Voltage: [%.2fv]", bc, time2Sleep, drift, vbat);
    Serial.println(a);
    iot.pub_msg(a);

}
void gotoSleep(int seconds2sleep=10) {
    Serial.printf("Going to sleep for %d [sec]", seconds2sleep);
    Serial.flush();
    delay(200);
    ESP.deepSleep(microsec2sec * seconds2sleep);
}
void calc_lastSleep_drift() {
    long lastwake=0;
    int bootCounter = 0;
    long curwake = now();

    FVAR_bootCounter.getValue(bootCounter);
    FVAR_bootCounter.setValue(++bootCounter);
    FVAR_lastboot.getValue(lastwake);
    FVAR_lastboot.setValue(curwake);

    int tdelta = curwake - lastwake;
    drift=tdelta-(time2Sleep+forceWake_sec);
    // Serial.printf("Last sleep was: %d, now is: %d, timedelta = %d [sec] drift is: %d [sec]\n", lastwake, curwake, tdelta, );
}

void setup()
{
    startIOTservices();
    calc_lastSleep_drift();
    start_adc();
    onWake_cb();
}
void loop()
{
    iot.looper();
    if (millis() > forceWake_sec*1000) {
        gotoSleep(time2Sleep);
    }
    delay(100);
}
