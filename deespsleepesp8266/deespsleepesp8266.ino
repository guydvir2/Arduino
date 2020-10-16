#include <myIOT.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_1.3"
#define SLEEP_MINUTES 30
#define FORCE_WAKE_SECONDS 15
#define USE_BAT true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false       // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "ESP8266_BAT_6V"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "SolarPower"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;
// ***************************

void startIOTservices()
{
    iot.useSerial = USE_SERIAL;
    iot.useWDT = USE_WDT;
    iot.useOTA = USE_OTA;
    iot.useResetKeeper = USE_RESETKEEPER;
    iot.resetFailNTP = USE_FAILNTP;
    strcpy(iot.deviceTopic, DEVICE_TOPIC);
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
const float v_logic = 3.3;
const int adc_res = 1023;
const float v_div_ratio = 0.591304348;
const float min_batVoltage = 3.0;
const float max_batVoltage = 4.2;

float vbat = 0;
float batRemain = 0;

void measure_bat_v(int x = 10)
{
    float analog_val = 0.0;
    for (int a = 0; a < x; a++)
    {
        analog_val += analogRead(A0);
        delay(50);
    }
    analog_val /= (float)x;

    vbat = (v_logic / (float)adc_res) * (analog_val / v_div_ratio);
    Serial.printf("Battery voltage: %.2f[v]\n", vbat);
}
void start_adc()
{
    pinMode(A0, INPUT);
    measure_bat_v(10);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~ DeepSleep ~~~~~~~~~~~~
#define MINUTES 60
#define microsec2sec 1000000ULL /* Conversion micro seconds to seconds */

const float driftFactor = 1.06; /* ESP8266 error correction for wake time - adds time to nominal sleep time*/
const int sleepTime = SLEEP_MINUTES * MINUTES;
FVars FVAR_bootClock("currentBoot");
FVars FVAR_bootCounter("bootCounter");
FVars FVAR_nextWakeClock("nextWake");

bool NTPconnect = false;
bool wifiConnect = false;
bool mqttConnect = false;
bool internetConnect = false;

int drift = 0;
int bootCount = 0;
int totalSleepTime = 0;
int nextsleep_duration = 0;

time_t wakeClock = 0;

// Wake functions
void onWake_cb()
{
    char a[100];
    int bc = 0;

    FVAR_bootCounter.getValue(bc);
    sprintf(a, "BootCounter: [#%d], sleepCycle: [%d sec], drift: [%d sec], Battery Voltage: [%.2fv]", bc, sleepTime, drift, vbat);
    Serial.println(a);
    iot.pub_msg(a);
}
void updateVars_afterWake()
{

    // Update Flash variables
    FVAR_bootCounter.getValue(bootCount);
    FVAR_bootCounter.setValue(++bootCount);
    FVAR_bootClock.setValue(wakeClock);
}
bool onWake_checkups()
{
    NTPconnect = iot.NTP_OK;
    internetConnect = iot.checkInternet();
    wifiConnect = WiFi.isConnected();
    mqttConnect = iot.mqttClient.connected();

    if (NTPconnect && internetConnect && wifiConnect && mqttConnect)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void wakeup_summary()
{
    wakeClock = now();
    time_t lastBoot = 0;
    time_t last_wakeClock = 0;

    FVAR_bootClock.getValue(lastBoot);
    FVAR_nextWakeClock.getValue(last_wakeClock);
    updateVars_afterWake();

    totalSleepTime = wakeClock - lastBoot - FORCE_WAKE_SECONDS;
    drift = wakeClock - last_wakeClock -(int)(millis()/1000);
}

// prepare to sleep~~~~
void nextSleepCalculation()
{
    time_t t = now();

    nextsleep_duration = sleepTime - (minute(t) * 60 + second(t)) % (sleepTime);
    nextsleep_duration = nextsleep_duration;
    FVAR_nextWakeClock.setValue(t + nextsleep_duration);

    // Serial.print("Next_time_duration is: ");
    // Serial.println(nextsleep_duration);
    // Serial.print("wake clock: ");
    // Serial.println(t + nextsleep_duration);
}
void gotoSleep(int seconds2sleep = 10)
{
    Serial.printf("Going to sleep for %d [sec]", seconds2sleep);
    Serial.flush();
    delay(200);
    ESP.deepSleep(microsec2sec * seconds2sleep);
}
void wait2Sleep()
{
    if (drift < 0 && drift > -30) /* wake up up to 30 sec earlier */
    {
        if (millis() > (FORCE_WAKE_SECONDS + abs(drift)) * 1000)
        {
            nextSleepCalculation();
            Serial.print("missed wake up by: ");
            Serial.println(drift);

            Serial.print("drift correction is: ");
            Serial.println((float)nextsleep_duration * (driftFactor));
            Serial.flush();

            gotoSleep(nextsleep_duration * driftFactor);
        }
    }
    else if (drift < -30) /* wake up more than 30 sec earlier */
    {
        Serial.print("missed wake up by: ");
        Serial.println(drift);
        Serial.println("going to sleep early");
        Serial.flush();

        gotoSleep(abs(drift));
    }
    else /* wake up after time - which is OK... sort of */
    {
        if (millis() > FORCE_WAKE_SECONDS * 1000)
        {
            nextSleepCalculation();
            Serial.print("missed wake up by: ");
            Serial.println(drift);

            Serial.print("drift correction is: ");
            Serial.println((float)nextsleep_duration * (driftFactor));
            Serial.flush();

            gotoSleep(nextsleep_duration * driftFactor);
        }
    }


    

}

String create_wakeStatus()
{
    StaticJsonDocument<300> doc;
    // Constansts
    doc["deviceName"] = DEVICE_TOPIC;
    doc["bootCount"] = bootCount;
    doc["forcedAwake"] = FORCE_WAKE_SECONDS;
    doc["NominalSleep"] = sleepTime;

    doc["totalSleep"] = totalSleepTime;
    doc["WakeErr"] = drift;

    // doc["WakeClock"] = wakeClock;
    doc["internet"] = internetConnect;
    doc["mqtt"] = mqttConnect;
    doc["NTP"] = NTPconnect;

    String output;
    serializeJson(doc, output);
    Serial.println(output);
    return output;
}
void sendSleepMQTT(char *msg)
{
    iot.mqttClient.publish("topic", msg, true);
}
void setup()
{
    startIOTservices();
#if USE_BAT
    start_adc();
#endif
    wakeup_summary();
    onWake_checkups();
    onWake_cb();
    create_wakeStatus();
}
void loop()
{
    iot.looper();
    wait2Sleep();
    delay(100);
}
