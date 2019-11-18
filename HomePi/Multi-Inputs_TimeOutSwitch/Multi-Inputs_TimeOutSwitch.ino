#include <myIOT.h>
#include <EEPROM.h>
#include <Arduino.h>

// ********** Names + Strings  ***********
// ~~~~~~~ MQTT Topics ~~~~~~                        // belonga rto myIOT
#define DEVICE_TOPIC "frontDoorLEDs"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "extLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// belongs to TELEGRAM

// ********** Sketch Services  ***********
#define VER              "WEMOS_4.5"
#define USE_INPUTS       true
#define IS_MOMENTARY     true  // is switch latch or momentary
#define ON_AT_BOOT       false // On or OFF at boot (Usually when using inputs, at boot/PowerOn - state should be off
#define USE_DAILY_TO     true
#define IS_SONOFF        false
#define HARD_REBOOT      false

#define USE_NOTIFY_TELE  false
#define USE_SENSOR       true
#define USE_IR_REMOTE    false

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false // Serial Monitor
#define USE_WDT          true  // watchDog resets
#define USE_OTA          true  // OTA updates
#define USE_RESETKEEPER  true // detect quick reboot and real reboots
#define USE_FAILNTP      true  // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT ~~~~~~
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************


// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      3*60 // mins for SW0
#define TIMEOUT_SW1      2*60 // mins

const int START_dailyTO[] = {18,0, 0};
const int END_dailyTO[]   = {0, 30, 0};

int TIMEOUTS[2]  = {TIMEOUT_SW0, TIMEOUT_SW1};
timeOUT timeOut_SW0("SW0", TIMEOUTS[0]);
#if NUM_SWITCHES == 2
timeOUT timeOut_SW1("SW1", TIMEOUTS[1]);
timeOUT *TO[] = {&timeOut_SW0, &timeOut_SW1};
#endif
#if NUM_SWITCHES == 1
timeOUT *TO[] = {&timeOut_SW0};
#endif
char *clockAlias = "Daily TimeOut";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~ HW Pins and States ~~~~
#if IS_SONOFF
#define RELAY1          12
#define RELAY2          5
#define INPUT1          0 // 0 for onBoard Button
#define INPUT2          14 // 14 for extButton
#define indic_LEDpin    13
#endif

#if !IS_SONOFF
#define RELAY1          D3 // <--- D3 most devices, but KitchenLEDs D2
#define RELAY2          D2
#define INPUT1          D7
#define INPUT2          D6
#define SENSOR_PIN      D1
#define indic_LEDpin    D4
#endif

#define IR_SENSOR_PIN   D5

byte relays[]       = {RELAY1, RELAY2};
byte inputs[]       = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW
#define ledON            HIGH

bool relState[NUM_SWITCHES];
bool last_relState[NUM_SWITCHES];
bool inputState[NUM_SWITCHES];
bool sensState[NUM_SWITCHES];
int rebootState         = 0;
bool checkrebootState   = true;
bool boot_overide[]     = {false, false};


// #################################  END CORE #################################



//  ######################### ADDITIONAL SERVICES ##############################

// ~~~~~ Use Reset Counter for hardReboot ~~~~
struct eeproms_storage {
        byte jump;
        byte val;
        byte val_cell;
        byte wcount;
        byte wcount_cell;
        bool hBoot;
};

eeproms_storage hReset_eeprom;

// ~~~~~~~~~~~~~ Sensor Switch ~~~~~~~~~~~~~~
#define MAX_NOTI_1HR           10
#define MIN_TIME_BETWEEN_NOTI  0.25 //in minutes
#define MIN_TIME_FIRST_DET     10    // sec
#define ADD_TIME_NEXT_DET      10   // sec

class SensorSwitch
{
#define SENS_IS_TRIGGERED LOW

private:
int _sensPin;
int _min_time;
int _to_time;

bool _inTriggerMode       = false;
long _detection_timestamp = 0;
long _timeout_counter     = 0;
int _calc_timeout         = 0;
int _time_from_detection  = 0;

public:
SensorSwitch(int sensPin, int min_time, int to_time) {
        _sensPin  = sensPin;
        _min_time = min_time;
        _to_time  = to_time;
}
void start() {
        pinMode(_sensPin, INPUT_PULLUP);
}

bool check_sensor() {
        _calc_timeout = (millis() - _timeout_counter) / 1000;

        if (_detection_timestamp != 0) {
                _time_from_detection = (millis() - _detection_timestamp) / 1000;
        }
        else {
                _time_from_detection = 0;
        }

        // HW senses
        if (digitalRead(_sensPin) == SENS_IS_TRIGGERED) {
                delay(50);
                if (digitalRead(_sensPin) == SENS_IS_TRIGGERED ) {
                        // First detection after TO  is OFF
                        if (_inTriggerMode == false && _detection_timestamp == 0 && _timeout_counter == 0) {
                                _inTriggerMode = true;
                                _detection_timestamp = millis();
                        }

                        // sensor senses again after sensor is not high - it starts T.O.
                        else if ( _inTriggerMode == false ) {
                                _timeout_counter = millis();
                        }

                        // very goes into T.O when sensor keeps HW sensing and time is greater than MIN_ON_TIME
                        else if (_inTriggerMode == true && _time_from_detection > _min_time && _detection_timestamp != 0 && _timeout_counter == 0) {
                                _inTriggerMode = false;
                        }
                }
        }

        // HW sense stops
        else {
                delay(50);
                if (digitalRead(_sensPin) == !SENS_IS_TRIGGERED) {
                        // Notify when HW sense ended
                        if (_inTriggerMode == true) {
                                _inTriggerMode = false;
                        }
                        // T.O has ended (greater than minimal time on detection)
                        else if (_inTriggerMode == false && _timeout_counter != 0 && _calc_timeout > _to_time) {
                                off_function();
                        }
                        // Minimal time on upon detection
                        else if ( _inTriggerMode == false && _time_from_detection > _min_time && _detection_timestamp != 0 && _timeout_counter == 0) {
                                off_function();
                        }
                }
        }

        if (_inTriggerMode == true || _detection_timestamp != 0 || _timeout_counter != 0) {
                return 1;
        }
        else {
                return 0;
        }
}
void off_function() {
        _detection_timestamp = 0;
        _timeout_counter = 0;
}
};

struct sensorNotify {
        unsigned long firstTime;
        unsigned long lastTime;
        byte nCounter;
        char msg[50];
};

char *sensor_notification_msg= "detection";
sensorNotify sensorNotify = {0, 0, 0, "detection"};

// ~~~~~~~~~~~ Using SMS Notification ~~~~~~~
#if USE_NOTIFY_TELE
char *Telegram_Nick = DEVICE_TOPIC;//"iotTest";
int time_check_messages = 1; //sec
myTelegram teleNotify(BOT_TOKEN, CHAT_ID, time_check_messages);
#endif

//~~~~~~~ IR Remote ~~~~~~~~
#if USE_IR_REMOTE
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint16_t kRecvPin        = IR_SENSOR_PIN;
const uint32_t kBaudRate       = 115200;
const uint16_t kMinUnknownSize = 12;
unsigned long key_value        = 0;

IRrecv irrecv(kRecvPin);
decode_results results;

#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #############################################################################



//  ############################## STRART CORE #################################

void switchIt (char *txt1, int sw_num, bool state, char *txt2 = "", bool show_timeout = true) {
        char msg [50], msg1[50], msg2[50], states[50], tempstr[50];
        char *word = {"Turned"};

        if (digitalRead(relays[sw_num]) != state || boot_overide[sw_num] == true) {
                digitalWrite(relays[sw_num], state);
                TO[sw_num]->convert_epoch2clock(now() + TO[sw_num]->remain(), now(), msg1, msg2);
                if (boot_overide[sw_num] == true) {
                        if(iot.mqtt_detect_reset == 1 || TO[sw_num]->remain() > 0) { //BOOT TIME ONLY for after quick boot
                                word = {"Resume"};
                                boot_overide[sw_num] = false;
                        }
                }
                sprintf(msg, "%s: Switch[#%d] %s[%s] %s", txt1, sw_num, word, state ? "ON" : "OFF", txt2);
                if (state == 1 && show_timeout) {
                        sprintf(msg2, "timeLeft[%s]", msg1);
                        strcat(msg, msg2);
                }

                iot.pub_msg(msg);

                sprintf(states, "");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(states, "[%s]", digitalRead(relays[i]) ? "ON" : "OFF");
                        // strcat(states, tempstr);
                        iot.pub_state(states, i);
                }
        }
}
void checkSwitch_Pressed (byte sw, bool momentary = true) {
        if (momentary) {
                if (digitalRead(inputs[sw]) == SwitchOn) {
                        delay(50);
                        if (digitalRead(inputs[sw]) == SwitchOn) {
                                char temp[20];
                                sprintf(temp,"Button: Switch [#%d] Turned [%s]", sw, digitalRead(relays[sw]) ? "OFF" : "ON");
                                if (digitalRead(relays[sw]) == RelayOn) {
                                        if (TO[sw]->remain() == 0) { // was ON but not in TO state
                                                switchIt("Button", sw, 0);
                                        }
                                        else {
                                                TO[sw]->endNow();
                                                iot.pub_msg(temp);
                                        }
                                }
                                else {
                                        iot.pub_msg(temp);
                                        TO[sw]->restart_to();
                                }
                                delay(500);
                        }
                }
        }
        else {
                if (digitalRead(inputs[sw]) != inputState[sw]) {
                        delay(50);
                        if (digitalRead(inputs[sw]) != inputState[sw]) {
                                inputState[sw] = digitalRead(inputs[sw]);
                                if (inputState[sw] == SwitchOn && digitalRead(relays[sw]) != RelayOn) { // turn in TO
                                        TO[sw]->restart_to();
                                }
                                else if( inputState[sw] != SwitchOn ) { // turn off
                                        if(TO[sw]->remain()>0) {  // turn off when in TO
                                                TO[sw]->endNow();
                                        }
                                        else if (digitalRead(relays[sw]) == RelayOn && TO[sw]->remain()==0) { // turn off when only ON
                                                switchIt("Button:", sw, 0,"", false);
                                        }
                                }
                        }
                }
        }
}
void startIOTservices() {
        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}
void startGPIOs() {
        for (int i = 0; i < NUM_SWITCHES; i++) {
                pinMode(relays[i], OUTPUT);
                if (USE_INPUTS) {
                        pinMode(inputs[i], INPUT_PULLUP);
                        inputState[i] = digitalRead(inputs[i]);
                }

                relState [i] = 0;
                last_relState [i] = 0;
        }
        pinMode(indic_LEDpin, OUTPUT);
}

// ~~~~~~ TimeOuts ~~~~~~~~~
void startTO(){
        for (int i=0; i<NUM_SWITCHES; i++) {
                TO[i]->begin(ON_AT_BOOT);
                if (USE_DAILY_TO) {
                        start_dailyTO(i);
                }
        }
}
void start_dailyTO(byte i){

        memcpy(TO[i]->dailyTO.on,START_dailyTO, sizeof(START_dailyTO));
        memcpy(TO[i]->dailyTO.off,END_dailyTO, sizeof(END_dailyTO));
        TO[i]->dailyTO.flag = USE_DAILY_TO;
        TO[i]->check_dailyTO_inFlash(TO[i]->dailyTO,i);
}
void notify_dailyTO(byte i){
  if (strcmp(TO[i]->dTO_pubMsg,"")!=0) {
          iot.pub_msg(TO[i]->dTO_pubMsg);
          sprintf(TO[i]->dTO_pubMsg,"");
  }
}
void TO_looper(byte i) {

        if (iot.mqtt_detect_reset != 2) {
                relState[i] = TO[i]->looper();
                notify_dailyTO(i);
                if (relState[i] != last_relState[i]) { // change state (ON <-->OFF)
                        switchIt("TimeOut", i, relState[i]);
                }
                last_relState[i] = relState[i];
        }
}

// ~~~~ MQTT Commands ~~~~~
void addiotnalMQTT(char *income_msg) {
        char msg_MQTT[150];
        char msg2[20];

        if      (strcmp(income_msg, "status") == 0) {
                giveStatus(msg_MQTT);
                iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(income_msg, "ver") == 0 ) {
                sprintf(msg_MQTT, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "ver #2: DailyTO[%d], UseInputs[%d], ON_AT_BOOT[%d], Use_Sensor[%d], Use_Telegram[%d], HardReboot[%d]",
                        USE_DAILY_TO, USE_INPUTS, ON_AT_BOOT, USE_SENSOR, USE_NOTIFY_TELE, HARD_REBOOT);
                iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(income_msg, "help") == 0) {
                sprintf(msg_MQTT, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "Help: Commands #2 - [remain, restartTO, timeout(x), endTO, updateTO(x), restoreTO, statusTO]");
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg_MQTT);
                sprintf(msg_MQTT, "Help: Commands #4 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
                iot.pub_msg(msg_MQTT);
        }
        else if (strcmp(income_msg, "flash") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(income_msg, "format") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.format();
        }
        else if (strcmp(income_msg, "all_off") == 0 ) {
                all_off("MQTT");

        }
        else {
                int num_parameters = iot.inline_read(income_msg);

                if (strcmp(iot.inline_param[1], "on") == 0 ) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 1, "", false);
                }
                else if (strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 0, "", false);
                }
                else if (strcmp(iot.inline_param[1], "timeout") == 0) {
                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg_MQTT);
                        sprintf(msg_MQTT, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "updateTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
                        sprintf(msg_MQTT, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                        iot.pub_msg(msg_MQTT);
                        delay(1000);
                        iot.notifyOffline();
                        iot.sendReset("TimeOut update");
                }
                else if (strcmp(iot.inline_param[1], "remain") == 0) {
                        if (TO[atoi(iot.inline_param[0])]->remain()>0) {
                                TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TO[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg_MQTT);
                                sprintf(msg_MQTT, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
                                iot.pub_msg(msg_MQTT);
                        }
                }
                else if (strcmp(iot.inline_param[1], "restartTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg_MQTT, "TimeOut: Switch [#%d] [Restart]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "statusTO") == 0) {
                        sprintf(msg_MQTT, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
                                "TimeOut", atoi(iot.inline_param[0]),
                                TIMEOUTS[atoi(iot.inline_param[0])],
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "endTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->endNow();
                        sprintf(msg_MQTT, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "restoreTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restore_to();
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg_MQTT, "TimeOut: Switch [#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
                        iot.pub_msg(msg_MQTT);
                        iot.notifyOffline();
                        iot.sendReset("Restore");
                }
                else if (strcmp(iot.inline_param[1], "on_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[0] = atoi(iot.inline_param[2]); //hours
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[1] = atoi(iot.inline_param[3]); // minutes
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[2] = atoi(iot.inline_param[4]); // seconds

                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch [#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[0], TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[2]);

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "off_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[0], TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[2]);

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "flag_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.flag = atoi(iot.inline_param[2]);
                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "useflash_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.useFlash = atoi(iot.inline_param[2]);
                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg_MQTT, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

                        iot.pub_msg(msg_MQTT);
                }
                else if (strcmp(iot.inline_param[1], "status_dailyTO") == 0) {
                        sprintf(msg_MQTT, "%s: Switch [#%d] {ON:%02d:%02d:%02d} {OFF:%02d:%02d:%02d} {Flag:%s} {Values:%s}",
                                clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[0],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[2],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[0],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[2],
                                TO[atoi(iot.inline_param[0])]->dailyTO.flag ? "ON" : "OFF",
                                TO[atoi(iot.inline_param[0])]->dailyTO.useFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg_MQTT);

                        Serial.println("IN STATUS_Daily LOOP");
                }
                else if (strcmp(iot.inline_param[1], "restart_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_dailyTO(TO[atoi(iot.inline_param[0])]->dailyTO);
                        sprintf(msg_MQTT, "%s: Switch[#%d] Resume daily Timeout", clockAlias, atoi(iot.inline_param[0]));
                        iot.pub_msg(msg_MQTT);
                }

                else{
                        if (strcmp(income_msg,"offline")!=0 && strcmp(income_msg,"online")!=0) {
                                sprintf(msg_MQTT,"Unrecognized Command: [%s]", income_msg);
                                iot.pub_err(msg_MQTT);
                        }
                }
                for (int n=0; n<=num_parameters -1; n++) {
                        sprintf(iot.inline_param[n],"");
                }
        }
}
void giveStatus(char *outputmsg){
        char t1 [50];
        char t2 [50];
        char t3 [50];

        sprintf(t3, "Status: ");
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (TO[i]->remain() > 0) {
                        TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), t2, t1);
                        sprintf(t1, "timeLeft[%s]", t2);
                }
                else {
                        sprintf(t1, "");
                }
                sprintf(t2, "Switch[#%d] [%s] %s ", i, digitalRead(relays[i]) ? "ON" : "OFF", t1);
                strcat(t3, t2);
        }
        sprintf(outputmsg,"%s",t3);
}
void all_off(char *from){
        char t[50];
        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (TO[i]->remain() > 0 && relays[i] == RelayOn) {
                        TO[i]->endNow();
                }
                else if (TO[i]->remain() == 0 && relays[i] == RelayOn) {
                        switchIt(from, i, false,"", false);
                }
                else if (TO[i]->remain() > 0 && relays[i] != RelayOn) {
                        TO[i]->endNow();
                }
        }
        sprintf(t,"All OFF: Received from %s",from);
        iot.pub_msg(t);
}

//  ######################### ADDITIONAL SERVICES ##############################
// ~~~~~~~~~~~ Telegram Notify ~~~~~~~
#if USE_NOTIFY_TELE
void telecmds(String in_msg, String from, String chat_id, char *snd_msg) {
        String command_set[] = {"whois_online", "status", "reset", "off", "on",
                                "timeout", "whoami","help"};
        byte num_commands = sizeof(command_set)/sizeof(command_set[0]);
        String comp_command[num_commands];
        char prefix[100], prefix2[100];
        char t1[50], t2[50];

        sprintf(snd_msg,""); // when not meeting any conditions, has to be empty

        from.toCharArray(t1,from.length()+1);
        in_msg.toCharArray(t2, in_msg.length()+1);

        sprintf(prefix,"/%s_", Telegram_Nick);
        sprintf(prefix2,"from user: %s\ndevice replies: %s\ncommand: %s\n~~~~~~~~~~~~~~~~~~~~\n ", t1,Telegram_Nick, t2);

        for (int i=0; i < num_commands; i++) {
                comp_command[i] = prefix;
                comp_command[i] += command_set[i];
                // Serial.print("command #");
                // Serial.print(i);
                // Serial.print(": ");
                // Serial.println(comp_command[i]);
        }

        if(in_msg=="/whois_online") {
                sprintf(snd_msg,"%s%s",prefix2, Telegram_Nick);
        }
        else if (in_msg==comp_command[1]) {
                giveStatus(t1);
                sprintf(snd_msg,"%s%s",prefix2, t1);
        } // status
        else if (in_msg==comp_command[2]) {
                sprintf(snd_msg,"%s",prefix2);
                iot.sendReset("Telegram");
        } // reset
        else if (in_msg==comp_command[3]) {
                all_off("Telegram");
                sprintf(snd_msg,"%sAll-Off signal was sent",prefix2);
        } // off
        else if (in_msg==comp_command[4]) {
        }
        else if (in_msg==comp_command[5]) {
                char m1[20];
                char m2[20];
                sprintf(snd_msg,"%s", prefix2);

                for(int i=0; i<NUM_SWITCHES; i++) {
                        if (TO[i]->remain() > 0) {
                                TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), m1, m2);
                                sprintf(t1,"Switch [#%d] already in TimeOut - timeLeft [%s]\n",i,m1);
                        }
                        else{
                                TO[i]->restart_to();
                                sprintf(t1,"Switch [#%d] restarting TimeOut\n",i);
                        }

                        strcat(snd_msg,t1);
                }
        } // timeout
        else if (in_msg==comp_command[6]) {
                sprintf(snd_msg,"%s~%s~ is %s",prefix2, Telegram_Nick, DEVICE_TOPIC);
        } // whoami
        else if (in_msg==comp_command[7]) {
                char t[50];
                sprintf(snd_msg,"%sCommands Available:\n", prefix2, Telegram_Nick);
                for (int i=0; i<num_commands; i++) {
                        command_set[i].toCharArray(t,30);
                        sprintf(t1,"%s\n",t);
                        strcat(snd_msg,t1);
                }

        } // all_commands
        Serial.print("in_msg: ");
        Serial.println(in_msg);
        Serial.print("snd_msg: ");
        Serial.println(snd_msg);

}
#endif

// ~~~~~~~~~~~ Using Sensor ~~~~~~~~~~~
void detectionBlink(byte sw, int duration=2000){
        unsigned long startTime = millis();
        bool last = digitalRead(relays[sw]);
        while (millis() - startTime < duration) {
                digitalWrite(relays[sw], RelayOn);
                delay(500);
                digitalWrite(relays[sw], !RelayOn);
                delay(50);
        }
        digitalWrite(relays[sw],last);

}
#if USE_SENSOR
SensorSwitch sensSW(SENSOR_PIN, MIN_TIME_FIRST_DET, ADD_TIME_NEXT_DET);
void notify_detection(byte sw){
        char time1[20];
        char date1[20];
        char comb[40];

        iot.return_clock(time1);
        iot.return_date(date1);
        sprintf(comb, "[%s %s] %s", date1, time1, sensorNotify.msg);

        if (millis() - sensorNotify.firstTime >= 1000*60*60 || sensorNotify.firstTime == 0) { // first time or reset counter after 1hr
                sensorNotify.firstTime = millis();
                sensorNotify.nCounter = 0;
        }
        if(millis() - sensorNotify.lastTime > 60*1000*MIN_TIME_BETWEEN_NOTI && sensorNotify.nCounter <MAX_NOTI_1HR) {
                detectionBlink(sw);
                sensorNotify.nCounter +=1;
                sensorNotify.lastTime = millis();
                sprintf(comb,"Sensor: Detection [#%d] in [%s]",sensorNotify.nCounter,DEVICE_TOPIC);
            #if USE_NOTIFY_TELE
                teleNotify.send_msg(comb);
            #endif
                iot.pub_msg(comb);
        }
}
void checkSensor_looper (byte sw) {
        bool current_sens_state = sensSW.check_sensor();

        if ( current_sens_state != sensState[sw]) {
                sensState[sw] = current_sens_state;
                if (current_sens_state) {
                        if (TO[sw]->remain() == 0 && digitalRead(relays[sw]) == !RelayOn) { // not in TO mode nor Relay is ON
                                switchIt("Sensor", sw, current_sens_state, "Detect", false);
                        }
                        else if (TO[sw]->remain() > 0 && digitalRead(relays[sw]) == RelayOn) {
                                notify_detection(sw);
                        }
                }
                else if (current_sens_state == false && TO[sw]->remain() == 0) {
                        switchIt("Sensor", sw, current_sens_state, "", false);
                }
        }
}
#endif

// ~~~~~ BOOT ASSIST SERVICES ~~~~~~~~~
#if HARD_REBOOT
void check_hardReboot(byte i = 1, byte threshold = 2) {
        // hReset_eeprom.jump = EEPROM.read(0);
        hReset_eeprom.val_cell    = hReset_eeprom.jump + i;
        hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);

        if (hReset_eeprom.val < threshold) {
                EEPROM.write(hReset_eeprom.val_cell, hReset_eeprom.val + 1);
                EEPROM.commit();
                hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);
                hReset_eeprom.hBoot = false;
        }
        else {
                hReset_eeprom.hBoot = true;
        }
}
#endif

void quickPwrON() {
        /*
           power on before iot starts,
           using the fact that endTimeOUT_inFlash contains value
           other than 0
         */

        /*
         # conditions in for loop:
           1) Has more time to go in TO
           2) ON_AT_BOOT defines to be ON at bootTime
           3) eeprom Reset counter forces to be ON_AT_BOOT
         */

        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (TO[i]->endTO_inFlash || ON_AT_BOOT || hReset_eeprom.hBoot) {
                        digitalWrite(relays[i], RelayOn);
                }
                else {
                        digitalWrite(relays[i], !RelayOn);
                }
        }
}
void recoverReset() {
        /*
           Using KeepAlive Service.
           This function determines if boot up caused be a regular PowerOn or caused
           by a power glich.
           The criteria is whther KeepAlive value was changed from
           "online" to "offline".
         */
        // Wait for indication if it was false reset(1) or
        char mqttmsg[30];
        rebootState = iot.mqtt_detect_reset;

        if (rebootState != 2) { // before getting online/offline MQTT state
                checkrebootState = false;
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (hReset_eeprom.hBoot && HARD_REBOOT) { // using HardReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> ForcedBoot. Restarting TimeOUT");
                        }
                        else if (rebootState == 0 && ON_AT_BOOT == true) {  // PowerOn - not a quickReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> NormalBoot & On-at-Boot. Restarting TimeOUT");
                        }
                        else if (TO[i]->looper() == 0) { // was not during TO
                                if (rebootState == 1) {
                                        iot.pub_err("--> PowerLoss Boot");
                                }
                                digitalWrite(relays[i], !RelayOn);
                                iot.pub_err("--> Stopping Quick-PowerON");

                        }
                        else{
                                iot.pub_err("--> Continue unfinished TimeOuts");
                                boot_overide[i] = true;
                        }
                }

                // Erases EEPROM value for HARD_REBOOT
                #if HARD_REBOOT
                EEPROM.write(hReset_eeprom.val_cell, 0);
                EEPROM.commit();
                #endif
        }
}

//~~~~~~~Run IR Remote ~~~~~~~~
void recvIRinputs() {
#if USE_IR_REMOTE
        char msg[50];

        if (irrecv.decode(&results)) {

                if (results.value == 0XFFFFFFFF)
                        results.value = key_value;

                switch (results.value) {
                case 0xFFA25D:
                        //Serial.println("CH-");
                        break;
                case 0xFF629D:
                        //Serial.println("CH");
                        iot.sendReset("RemoteControl");
                        break;
                case 0xFFE21D:
                        //Serial.println("CH+");
                        break;
                case 0xFF22DD:
                        //Serial.println("|<<");
                        break;
                case 0xFF02FD:
                        //Serial.println(">>|");
                        break;
                case 0xFFC23D:
                        //Serial.println(">|");
                        break;
                case 0xFFE01F:
                        //Serial.println("-");
                        TO[0]->endNow();
                        sprintf(msg, "TimeOut: IRremote[Abort]");
                        iot.pub_msg(msg);
                        break;
                case 0xFFA857:
                        //Serial.println("+");
                        TO[0]->restart_to();
                        sprintf(msg, "TimeOut: IRremote[Restart]");
                        iot.pub_msg(msg);
                        break;
                case 0xFF906F:
                        //Serial.println("EQ");
                        break;
                case 0xFF6897:
                        //Serial.println("0");
                        break;
                case 0xFF9867:
                        //Serial.println("100+");
                        break;
                case 0xFFB04F:
                        //Serial.println("200+");
                        break;
                case 0xFF30CF:
                        // blinker_state = !blinker_state;
                        // sprintf(msg, "IRremote: [Blinker], [%s]", blinker_state ? "ON" : "OFF");
                        // iot.pub_msg(msg);
                        break;
                case 0xFF18E7:
                        // strobe_state = !strobe_state;
                        // sprintf(msg, "Switch: [Strobe], [%s]", strobe_state ? "ON" : "OFF");
                        // iot.pub_msg(msg);
                        break;
                case 0xFF7A85:
                        break;
                case 0xFF10EF:
                        //change_color(4);
                        break;
                case 0xFF38C7:
                        //change_color(5);
                        break;
                case 0xFF5AA5:
                        //Serial.println("6");
                        //                        change_color(6);
                        break;
                case 0xFF42BD:
                        //Serial.println("7");
                        //                        change_color(7);
                        break;
                case 0xFF4AB5:
                        //Serial.println("8");
                        //                        change_color(8);
                        break;
                case 0xFF52AD:
                        //Serial.println("9");
                        //                        change_color(9);
                        break;
                }
                key_value = results.value;
                irrecv.resume();
        }
#endif
}
void start_IR() {
#if USE_IR_REMOTE
#if DECODE_HASH
        // Ignore messages with less than minimum on or off pulses.
        irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
        irrecv.enableIRIn(); // Start the receiver
#endif
}

// ########################### END ADDITIONAL SERVICE ##########################


void setup() {

        #if HARD_REBOOT
        EEPROM.begin(1024);
        check_hardReboot();
        #endif

        startGPIOs();
        quickPwrON();
        startIOTservices();
        startTO();

        #if USE_SENSOR
        sensSW.start();
        #endif

        #if USE_NOTIFY_TELE
        teleNotify.begin(telecmds);
        #endif

        #if USE_IR_REMOTE
        start_IR();
        #endif
}
void loop() {
        iot.looper();

        #if USE_RESETKEEPER
        if (checkrebootState == true) {
                recoverReset();
        }
        #endif

        for (int i = 0; i < NUM_SWITCHES; i++) {
                if (USE_INPUTS) {
                        checkSwitch_Pressed(i,IS_MOMENTARY);
                }
                TO_looper(i);
        }
        digitalWrite(indic_LEDpin, !relState[0]);


        #if USE_NOTIFY_TELE
        teleNotify.looper();
        #endif

        #if USE_SENSOR
        checkSensor_looper(0);
        #endif

        #if USE_IR_REMOTE
        recvIRinputs(); // IR signals
        #endif

        delay(50);
}
