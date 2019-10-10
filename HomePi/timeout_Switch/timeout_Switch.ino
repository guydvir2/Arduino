/* For SonOff Devices:
   1) Upload using Generic ESP8266
   2) Change default 1024kB (64kB SPIFFS ) ram
   <<<<  !!! MUST!! >>>>
   otherwise OTA will not be loaded next time
 */

#include <myIOT.h>
#include <Arduino.h>
#include <EEPROM.h>


// ********** Sketch Services  ***********
#define VER              "Sonoff_4.0"
#define USE_INPUTS       false
#define ON_AT_BOOT       true // On or OFF at boot (Usually when using inputs, at boot/PowerOn - state should be off
#define USE_DAILY_TO     true
#define IS_SONOFF        true
#define HARD_REBOOT      true

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false // Serial Monitor
#define USE_WDT          true  // watchDog resets
#define USE_OTA          true  // OTA updates
#define USE_RESETKEEPER  true // detect quick reboot and real reboots
#define USE_FAILNTP      true  // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "frontDoor"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   "extLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************


// ********** TimeOut Time vars  ***********
#define NUM_SWITCHES     1
#define TIMEOUT_SW0      2*60 // mins for SW0
#define TIMEOUT_SW1      2*60 // mins

const int START_dailyTO[] = {18,0,0};
const int END_dailyTO[]   = {2,30,0};

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

// ~~~~ HW Pins and Statdes ~~~~
#if IS_SONOFF
#define RELAY1          12
#define RELAY2          5
#define INPUT1          0
#define INPUT2          14
#endif

#if !IS_SONOFF
#define RELAY1          D2
#define RELAY2          5
#define INPUT1          9
#define INPUT2          3
#endif

#define indic_LEDpin          13


byte relays[]  = {RELAY1, RELAY2};
byte inputs[]  = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW

bool relState[NUM_SWITCHES];
bool last_relState[NUM_SWITCHES];
bool inputState[NUM_SWITCHES];

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

int rebootState         = 0;
bool checkrebootState   = true;
bool boot_overide       = true;



void switchIt (char *txt1, int sw_num, bool state, char *txt2 = "", bool show_timeout = true){
        char msg [50], msg1[50], msg2[50], states[50], tempstr[50];
        char *word = {"Turned"};

        if (digitalRead(relays[sw_num]) != state || boot_overide == true) {
                digitalWrite(relays[sw_num], state);
                TO[sw_num]->convert_epoch2clock(now() + TO[sw_num]->remain(), now(), msg1, msg2);
                if (boot_overide == true && iot.mqtt_detect_reset == 1) { //BOOT TIME ONLY for after quick boot
                        word = {"Resume"};
                        boot_overide = false;
                }
                sprintf(msg, "%s: Switch[#%d] %s[%s] %s", txt1, sw_num, word, state ? "ON" : "OFF", txt2);
                if (state == 1 && show_timeout) {
                        sprintf(msg2, "timeLeft[%s]", msg1);
                        strcat(msg, msg2);
                }

                iot.pub_msg(msg);

                sprintf(states, "");
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        sprintf(tempstr, "[%s]", !digitalRead(relays[i]) ? "ON" : "OFF");
                        strcat(states, tempstr);
                }
                iot.pub_state(states);
        }
}
void checkSwitch_Pressed (byte sw, bool momentary = true) {
        if (momentary) {
                if (digitalRead(inputs[sw]) == LOW) {
                        delay(50);
                        if (digitalRead(inputs[sw]) == LOW) {
                                char temp[20];
                                sprintf(temp,"Button: Switch [#%d] Turned [%s]", sw, digitalRead(relays[sw]) ? "OFF" : "ON");
                                if (digitalRead(relays[sw]) == RelayOn) {
                                        iot.pub_msg(temp);
                                        TO[sw]->endNow();
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
                                if (digitalRead(inputs[sw]) == SwitchOn) {
                                        TO[sw]->restart_to();
                                }
                                else {
                                        TO[sw]->endNow();
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
                pinMode(inputs[i], INPUT_PULLUP);

                if (USE_INPUTS) {
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
        memcpy(TO[i]->dailyTO.on,START_dailyTO, 9);
        memcpy(TO[i]->dailyTO.off,END_dailyTO, 9);
        TO[i]->dailyTO.flag = USE_DAILY_TO;
        TO[i]->check_dailyTO_inFlash(TO[i]->dailyTO,i);
}
void TO_looper(byte i) {
        char msg_t[50], msg[50];

        if (iot.mqtt_detect_reset != 2) {
                relState[i] = TO[i]->looper();
                if (relState[i] != last_relState[i]) { // change state (ON <-->OFF)
                        switchIt("TimeOut", i, relState[i]);
                }
                last_relState[i] = relState[i];
        }
}

// ~~~~ MQTT Commands ~~~~~
void addiotnalMQTT(char *incoming_msg) {
        char msg[150];
        char msg2[20];
        if      (strcmp(incoming_msg, "status") == 0) {
                for (int i = 0; i < NUM_SWITCHES; i++) {
                        if (TO[i]->remain() > 0) {
                                TO[i]->convert_epoch2clock(now() + TO[i]->remain(), now(), msg, msg2);
                                sprintf(msg2, "timeLeft[%s]", msg);
                        }
                        else {
                                sprintf(msg2, "");
                        }
                        sprintf(msg, "Status: Switch[#%d] [%s] %s", i, digitalRead(relays[i]) ? "ON" : "OFF", msg2);
                        iot.pub_msg(msg);
                }
        }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
                sprintf(msg, "ver #2: DailyTO[%d], UseInputs[%d], ON_AT_BOOT[%d], HardReboot[%d]", USE_DAILY_TO, USE_INPUTS, ON_AT_BOOT, HARD_REBOOT);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0) {
                sprintf(msg, "Help: Commands #1 - [on, off, flash, format]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [remain, restartTO, timeout(x), endTO, updateTO(x), restoreTO, statusTO]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #3 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #4 - [off_dailyTO, on_dailyTO, flag_dailyTO, useflash_dailyTO, status_dailyTO]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "flash") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.printFile();
        }
        else if (strcmp(incoming_msg, "format") == 0 ) {
                TO[0]->inCodeTimeOUT_inFlash.format();
        }
        else {
                iot.inline_read(incoming_msg);

                if (strcmp(iot.inline_param[1], "on") == 0 ) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 1);
                }
                else if (strcmp(iot.inline_param[1], "off") == 0) {
                        switchIt("MQTT", atoi(iot.inline_param[0]), 0);
                }
                else if (strcmp(iot.inline_param[1], "timeout") == 0) {
                        TO[atoi(iot.inline_param[0])]->setNewTimeout(atoi(iot.inline_param[2]));
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + atoi(iot.inline_param[2]) * 60, now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[#%d] one-time TimeOut %s", atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "updateTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->updateTOinflash(atoi(iot.inline_param[2]));
                        sprintf(msg, "TimeOut: Switch [%d] Updated in flash to [%d min.]", atoi(iot.inline_param[0]), atoi(iot.inline_param[2]));
                        iot.pub_msg(msg);
                        delay(1000);
                        iot.notifyOffline();
                        iot.sendReset("TimeOut update");
                }
                else if (strcmp(iot.inline_param[1], "remain") == 0) {
                        TO[atoi(iot.inline_param[0])]->convert_epoch2clock(now() + TO[atoi(iot.inline_param[0])]->remain(), now(), msg2, msg);
                        sprintf(msg, "TimeOut: Switch[#%d] Remain [%s]", atoi(iot.inline_param[0]), msg2);
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "restartTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg, "TimeOut: Switch [#%d] [Restart]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "statusTO") == 0) {
                        sprintf(msg, "%s: Switch [#%d] {inCode: [%d] mins} {Flash: [%d] mins}, {Active: [%s]}",
                                "TimeOut", atoi(iot.inline_param[0]),
                                TIMEOUTS[atoi(iot.inline_param[0])],
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash,
                                TO[atoi(iot.inline_param[0])]->updatedTO_inFlash ? "Flash" : "inCode" );
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "endTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->endNow();
                        sprintf(msg, "TimeOut: Switch[#%d] [Abort]", atoi(iot.inline_param[0]));
                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "restoreTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->restore_to();
                        TO[atoi(iot.inline_param[0])]->restart_to();
                        sprintf(msg, "TimeOut: Switch [#%d], Restore hardCoded Value [%d mins.]", atoi(iot.inline_param[0]), TIMEOUT_SW0);
                        iot.pub_msg(msg);
                        iot.notifyOffline();
                        iot.sendReset("Restore");
                }
                else if (strcmp(iot.inline_param[1], "on_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[0] = atoi(iot.inline_param[2]); //hours
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[1] = atoi(iot.inline_param[3]); // minutes
                        TO[atoi(iot.inline_param[0])]->dailyTO.on[2] = atoi(iot.inline_param[4]); // seconds

                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Switch [#%d] [ON] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[0], TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[2]);

                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "off_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[0] = atoi(iot.inline_param[2]); //hours
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[1] = atoi(iot.inline_param[3]); // minutes
                        TO[atoi(iot.inline_param[0])]->dailyTO.off[2] = atoi(iot.inline_param[4]); // seconds

                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Switch [#%d] [OFF] updated [%02d:%02d:%02d]", clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[0], TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[2]);

                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "flag_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.flag = atoi(iot.inline_param[2]);
                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "ON" : "OFF");

                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "useflash_dailyTO") == 0) {
                        TO[atoi(iot.inline_param[0])]->dailyTO.useFlash = atoi(iot.inline_param[2]);
                        TO[atoi(iot.inline_param[0])]->store_dailyTO_inFlash(TO[atoi(iot.inline_param[0])]->dailyTO, atoi(iot.inline_param[0]));

                        sprintf(msg, "%s: Switch[#%d] using [%s] values", clockAlias,
                                atoi(iot.inline_param[0]), atoi(iot.inline_param[2]) ? "Flash" : "Code");

                        iot.pub_msg(msg);
                }
                else if (strcmp(iot.inline_param[1], "status_dailyTO") == 0) {
                        sprintf(msg, "%s: Switch [#%d] {ON, %02d:%02d:%02d} {OFF, %02d:%02d:%02d} {Flag: %s}",
                                clockAlias, atoi(iot.inline_param[0]),
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[0],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.on[2],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[0],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[1],
                                TO[atoi(iot.inline_param[0])]->dailyTO.off[2],
                                TO[atoi(iot.inline_param[0])]->dailyTO.flag ? "ON" : "OFF" );
                        TO[atoi(iot.inline_param[0])]->dailyTO_inFlash.printFile();
                        iot.pub_msg(msg);
                }
        }
}


//  ######################### ADDITIONAL SERVICES ##############################

// ~~~~~ BOOT ASSIST SERVICES ~~~~~~~~~
#if HARD_REBOOT
void check_hardReboot(byte i = 1, byte threshold = 2) {
        hReset_eeprom.jump = EEPROM.read(0);
        hReset_eeprom.val_cell    = hReset_eeprom.jump + i;
        hReset_eeprom.val = EEPROM.read(hReset_eeprom.val_cell);

        if (hReset_eeprom.val < threshold) {
                EEPROM.write(hReset_eeprom.val_cell, hReset_eeprom.val + 1);
                EEPROM.commit();
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
                        if (rebootState == 0 && ON_AT_BOOT == true) {  // PowerOn - not a quickReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> NormalBoot. On-at-Boot");
                        }
                        #if HARD_REBOOT
                        else if (hReset_eeprom.hBoot ) { // using HardReboot
                                TO[i]->restart_to();
                                iot.pub_err("--> ForcedBoot");
                        }
                        #endif
                        else if (rebootState == 1) {
                                iot.pub_err("--> PowerLoss Boot");
                        }
                        else if (TO[i]->looper() == 0) {
                                digitalWrite(relays[i], !RelayOn);
                                iot.pub_err("--> Stopping Quick-PowerON");
                        }

                        // else { // prevent quick boot to restart after succsefull end
                        //         if (TO[i]->begin(false) == 0) { // if ON_AT_BOOT == true turn off if not needed
                        //                 digitalWrite(relays[i], !RelayOn);
                        //                 Serial.println("AD");
                        //         }
                        //         iot.pub_err("--> Quick-Reset");
                        // }
                }
        }
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

#if HARD_REBOOT
        EEPROM.write(hReset_eeprom.val_cell, 0);
        EEPROM.commit();
#endif

}
void loop() {
        iot.looper();

        for (int i = 0; i < NUM_SWITCHES; i++) {
                TO_looper(i);
                if (USE_INPUTS == true) {
                        checkSwitch_Pressed(i);
                }
        }
       digitalWrite(indic_LEDpin, !relState[0]);

#if USE_RESETKEEPER
        if (checkrebootState == true) {
                recoverReset();
        }
#endif

        delay(100);
}
